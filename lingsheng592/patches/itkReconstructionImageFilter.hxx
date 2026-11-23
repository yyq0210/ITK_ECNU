/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkReconstructionImageFilter_hxx
#define itkReconstructionImageFilter_hxx

#include "itkMath.h"
#include "itkConstantBoundaryCondition.h"
#include "itkConnectedComponentAlgorithm.h"

#include "itkConstantPadImageFilter.h"
#include "itkCropImageFilter.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <type_traits>
#include <vector>

namespace itk
{

// 4-connected raster reconstruction for scalar 2D images.
// Same propagation order as the neighborhood-iterator path:
// forward previous neighbors (-x, -y), reverse later neighbors (+x, +y),
// then a FIFO over the full face neighborhood (-x, +x, -y, +y).
// Returns 0 if the marker/mask precondition fails, 1 on success, 2 to fall back.
inline void
ReconstructionRasterNote(const char * why)
{
  const char * flag = std::getenv("ITK_RECON_FAST_DEBUG");
  if (flag == nullptr || flag[0] == '\0')
  {
    return;
  }
  std::cerr << "recon-fast fallback: " << why << std::endl;
}

template <typename TImage, typename TCompare>
static int
ReconstructionRaster2D(const TImage *                  markerImage,
                       const TImage *                  maskImage,
                       TImage *                        output,
                       typename TImage::PixelType      markerValue,
                       TCompare                        compare,
                       ProgressReporter &              progress)
{
  using Pixel = typename TImage::PixelType;

  const auto region = markerImage->GetLargestPossibleRegion();
  if (region.GetIndex()[0] != 0 || region.GetIndex()[1] != 0)
  {
    ReconstructionRasterNote("index");
    return 2;
  }
  if (markerImage->GetBufferedRegion() != region || maskImage->GetBufferedRegion() != region ||
      output->GetBufferedRegion() != region)
  {
    ReconstructionRasterNote("buffered-region");
    return 2;
  }
  if (maskImage->GetLargestPossibleRegion().GetSize() != region.GetSize() ||
      output->GetLargestPossibleRegion().GetSize() != region.GetSize())
  {
    ReconstructionRasterNote("size");
    return 2;
  }

  const auto * markerStride = markerImage->GetOffsetTable();
  const auto * maskStride = maskImage->GetOffsetTable();
  const auto * outStride = output->GetOffsetTable();
  if (markerStride[0] != 1 || maskStride[0] != 1 || outStride[0] != 1)
  {
    ReconstructionRasterNote("stride");
    return 2;
  }

  const std::ptrdiff_t sx = static_cast<std::ptrdiff_t>(region.GetSize()[0]);
  const std::ptrdiff_t sy = static_cast<std::ptrdiff_t>(region.GetSize()[1]);
  if (sx <= 0 || sy <= 0)
  {
    return 1;
  }

  const std::ptrdiff_t pw = sx + 2;
  const std::ptrdiff_t ph = sy + 2;
  std::vector<Pixel>   marker(static_cast<std::size_t>(pw * ph), markerValue);
  std::vector<Pixel>   mask(static_cast<std::size_t>(pw * ph), markerValue);

  const Pixel * markerBuf = markerImage->GetBufferPointer();
  const Pixel * maskBuf = maskImage->GetBufferPointer();
  for (std::ptrdiff_t y = 0; y < sy; ++y)
  {
    std::copy_n(markerBuf + y * markerStride[1], sx, marker.data() + (y + 1) * pw + 1);
    std::copy_n(maskBuf + y * maskStride[1], sx, mask.data() + (y + 1) * pw + 1);
  }

  Pixel *       m = marker.data();
  const Pixel * k = mask.data();

  for (std::ptrdiff_t y = 1; y <= sy; ++y)
  {
    Pixel *       row = m + y * pw;
    const Pixel * krow = k + y * pw;
    for (std::ptrdiff_t x = 1; x <= sx; ++x)
    {
      Pixel V = row[x];
      const Pixel iV = krow[x];
      if (compare(V, iV))
      {
        return 0;
      }
      Pixel VN = row[x - 1];
      if (compare(VN, V))
      {
        V = VN;
        row[x] = V;
      }
      VN = row[x - pw];
      if (compare(VN, V))
      {
        V = VN;
        row[x] = V;
      }
      if (compare(V, iV))
      {
        row[x] = iV;
      }
      progress.CompletedPixel();
    }
  }

  std::queue<std::ptrdiff_t> fifo;
  for (std::ptrdiff_t y = sy; y >= 1; --y)
  {
    Pixel *       row = m + y * pw;
    const Pixel * krow = k + y * pw;
    for (std::ptrdiff_t x = sx; x >= 1; --x)
    {
      Pixel V = row[x];
      Pixel VN = row[x + 1];
      if (compare(VN, V))
      {
        V = VN;
        row[x] = V;
      }
      VN = row[x + pw];
      if (compare(VN, V))
      {
        V = VN;
        row[x] = V;
      }
      const Pixel iV = krow[x];
      if (compare(V, iV))
      {
        row[x] = iV;
        V = iV;
      }

      const std::ptrdiff_t I = y * pw + x;
      VN = row[x + 1];
      Pixel iN = krow[x + 1];
      if (compare(V, VN) && compare(iN, VN))
      {
        fifo.push(I);
      }
      else
      {
        VN = m[I + pw];
        iN = k[I + pw];
        if (compare(V, VN) && compare(iN, VN))
        {
          fifo.push(I);
        }
      }
      progress.CompletedPixel();
    }
  }

  const std::ptrdiff_t dx[4] = { -1, 1, 0, 0 };
  const std::ptrdiff_t dy[4] = { 0, 0, -1, 1 };
  while (!fifo.empty())
  {
    const std::ptrdiff_t I = fifo.front();
    fifo.pop();
    const std::ptrdiff_t x = I % pw;
    const std::ptrdiff_t y = I / pw;
    const Pixel          V = m[I];
    for (int n = 0; n < 4; ++n)
    {
      const std::ptrdiff_t nx = x + dx[n];
      const std::ptrdiff_t ny = y + dy[n];
      if (nx < 0 || ny < 0 || nx >= pw || ny >= ph)
      {
        continue;
      }
      const std::ptrdiff_t J = ny * pw + nx;
      const Pixel          VN = m[J];
      const Pixel          iN = k[J];
      if (compare(V, VN) && Math::NotAlmostEquals(iN, VN))
      {
        if (compare(iN, V))
        {
          m[J] = V;
        }
        else
        {
          m[J] = iN;
        }
        fifo.push(J);
      }
    }
    progress.CompletedPixel();
  }

  Pixel * outBuf = output->GetBufferPointer();
  for (std::ptrdiff_t y = 0; y < sy; ++y)
  {
    std::copy_n(m + (y + 1) * pw + 1, sx, outBuf + y * outStride[1]);
  }
  return 1;
}

template <typename TInputImage, typename TOutputImage, typename TCompare>
ReconstructionImageFilter<TInputImage, TOutputImage, TCompare>::ReconstructionImageFilter()
{
  m_FullyConnected = false;
  m_UseInternalCopy = true;

  this->SetPrimaryInputName("MarkerImage");
  this->AddRequiredInputName("MaskImage", 1);
}

template <typename TInputImage, typename TOutputImage, typename TCompare>
void
ReconstructionImageFilter<TInputImage, TOutputImage, TCompare>::GenerateInputRequestedRegion()
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // get pointers to the inputs
  MarkerImagePointer markerPtr = const_cast<MarkerImageType *>(this->GetInput(0));

  MaskImagePointer maskPtr = const_cast<MaskImageType *>(this->GetInput(1));

  if (!markerPtr || !maskPtr)
  {
    return;
  }
  markerPtr->SetRequestedRegion(markerPtr->GetLargestPossibleRegion());
  maskPtr->SetRequestedRegion(maskPtr->GetLargestPossibleRegion());
}

template <typename TInputImage, typename TOutputImage, typename TCompare>
void
ReconstructionImageFilter<TInputImage, TOutputImage, TCompare>::EnlargeOutputRequestedRegion(DataObject *)
{
  this->GetOutput()->SetRequestedRegion(this->GetOutput()->GetLargestPossibleRegion());
}

// a version that takes a padded copy of mask and marker
template <typename TInputImage, typename TOutputImage, typename TCompare>
void
ReconstructionImageFilter<TInputImage, TOutputImage, TCompare>::GenerateData()
{
  // Allocate the output
  this->AllocateOutputs();
  // there are 2 passes that use all pixels and a 3rd that uses some
  // subset of the pixels. We'll just pretend that the third pass
  // takes the same as each of the others. Is it OK to update more
  // often than pixels?
  ProgressReporter progress(this, 0, this->GetOutput()->GetRequestedRegion().GetNumberOfPixels() * 3);

  TCompare compare;

  MarkerImageConstPointer markerImage = this->GetMarkerImage();
  MaskImageConstPointer   maskImage = this->GetMaskImage();
  OutputImagePointer      output = this->GetOutput();

  // mask and marker must have the same size
  if (this->GetMarkerImage()->GetRequestedRegion().GetSize() != this->GetMaskImage()->GetRequestedRegion().GetSize())
  {
    itkExceptionMacro("Marker and mask must have the same size.");
  }

  if constexpr (InputImageType::ImageDimension == 2 && std::is_same<InputImageType, OutputImageType>::value &&
                std::is_arithmetic<InputImagePixelType>::value)
  {
    if (!m_FullyConnected && m_UseInternalCopy)
    {
      const int rasterStatus = ReconstructionRaster2D<InputImageType, TCompare>(
        markerImage.GetPointer(), maskImage.GetPointer(), output.GetPointer(), m_MarkerValue, compare, progress);
      if (rasterStatus == 0)
      {
        if (compare(static_cast<OutputImagePixelType>(0), static_cast<OutputImagePixelType>(1)))
        {
          itkExceptionMacro("Marker pixels must be <= mask pixels.");
        }
        else
        {
          itkExceptionMacro("Marker pixels must be >= mask pixels.");
        }
      }
      if (rasterStatus == 1)
      {
        return;
      }
    }
  }

  // create padded versions of the marker image and the mask image
  using PadType = typename itk::ConstantPadImageFilter<InputImageType, InputImageType>;

  MarkerImageConstPointer markerImageP;
  MaskImageConstPointer   maskImageP;

  ISizeType padSize;

  if (m_UseInternalCopy)
  {
    auto MaskPad = PadType::New();
    auto MarkerPad = PadType::New();
    padSize.Fill(1);

    MaskPad->SetConstant(m_MarkerValue);
    MarkerPad->SetConstant(m_MarkerValue);
    MaskPad->SetPadLowerBound(padSize);
    MaskPad->SetPadUpperBound(padSize);
    MarkerPad->SetPadLowerBound(padSize);
    MarkerPad->SetPadUpperBound(padSize);

    MaskPad->SetInput(maskImage);
    MarkerPad->SetInput(markerImage);
    MaskPad->Update();
    MarkerPad->Update();

    markerImageP = MarkerPad->GetOutput();
    maskImageP = MaskPad->GetOutput();
  }
  else
  {
    maskImageP = this->GetMaskImage();
    InputIteratorType  inIt(markerImage, output->GetRequestedRegion());
    OutputIteratorType outIt(output, output->GetRequestedRegion());
    // copy marker to output - isn't there a better way?
    while (!outIt.IsAtEnd())
    {
      MarkerImagePixelType currentValue = inIt.Get();
      outIt.Set(static_cast<OutputImagePixelType>(currentValue));
      ++inIt;
      ++outIt;
    }
    markerImageP = output;
  }

  // declare our queue type
  using FifoType = typename std::queue<OutputImageIndexType>;
  FifoType IndexFifo;

  NOutputIterator   outNIt;
  InputIteratorType mskIt;
  CNInputIterator   mskNIt;
  ISizeType         kernelRadius;
  kernelRadius.Fill(1);
  if (m_UseInternalCopy)
  {
    FaceCalculatorType faceCalculator;

    FaceListTypeIt fit;

    FaceListType faceList = faceCalculator(maskImageP, maskImageP->GetLargestPossibleRegion(), kernelRadius);
    // we will only be processing the body region
    fit = faceList.begin();
    // must be a better way of doing this
    NOutputIterator tt(kernelRadius, markerImageP, *fit);
    outNIt = tt;

    InputIteratorType ttt(maskImageP, *fit);
    mskIt = ttt;
    CNInputIterator tttt(kernelRadius, maskImageP, *fit);
    mskNIt = tttt;
  }
  else
  {
    NOutputIterator tt(kernelRadius, markerImageP, output->GetRequestedRegion());
    outNIt = tt;

    InputIteratorType ttt(maskImageP, output->GetRequestedRegion());
    mskIt = ttt;
    CNInputIterator tttt(kernelRadius, maskImageP, output->GetRequestedRegion());
    mskNIt = tttt;
  }

  setConnectivityPrevious(&outNIt, m_FullyConnected);

  ConstantBoundaryCondition<OutputImageType> oBC;
  oBC.SetConstant(m_MarkerValue);
  outNIt.OverrideBoundaryCondition(&oBC);

  mskIt.GoToBegin();
  // scan in forward raster order
  for (outNIt.GoToBegin(), mskIt.GoToBegin(); !outNIt.IsAtEnd(); ++outNIt, ++mskIt)
  {
    InputImagePixelType V = outNIt.GetCenterPixel();
    auto                iV = static_cast<OutputImagePixelType>(mskIt.Get());

    // be sure that the pixels in the images follow the preconditions
    if (compare(V, iV))
    {
      if (compare(0, 1))
      {
        itkExceptionMacro("Marker pixels must be <= mask pixels.");
      }
      else
      {
        itkExceptionMacro("Marker pixels must be >= mask pixels.");
      }
    }

    // visit the previous neighbours
    typename NOutputIterator::ConstIterator sIt;
    for (sIt = outNIt.Begin(); !sIt.IsAtEnd(); ++sIt)
    {
      InputImagePixelType VN = sIt.Get();
      if (compare(VN, V))
      {
        outNIt.SetCenterPixel(VN);
        V = VN;
      }
    }

    // this step clamps to the mask
    if (compare(V, iV))
    {
      outNIt.SetCenterPixel(iV);
    }

    progress.CompletedPixel();
  }

  // now for the reverse raster order pass
  // reset the neighborhood
  setConnectivityLater(&outNIt, m_FullyConnected);
  outNIt.OverrideBoundaryCondition(&oBC);
  outNIt.GoToEnd();
  // mskIt.GoToEnd();

  ConstantBoundaryCondition<InputImageType> iBC;
  iBC.SetConstant(m_MarkerValue);

  setConnectivityLater(&mskNIt, m_FullyConnected);
  mskNIt.OverrideBoundaryCondition(&iBC);

  typename NOutputIterator::IndexListType                 oIndexList, mIndexList;
  typename NOutputIterator::IndexListType::const_iterator oLIt, mLIt;

  oIndexList = outNIt.GetActiveIndexList();
  mIndexList = mskNIt.GetActiveIndexList();

  mskNIt.GoToEnd();
  while (!outNIt.IsAtBegin())
  {
    --outNIt;
    --mskNIt;
    InputImagePixelType                     V = outNIt.GetCenterPixel();
    typename NOutputIterator::ConstIterator sIt;
    for (sIt = outNIt.Begin(); !sIt.IsAtEnd(); ++sIt)
    {
      InputImagePixelType VN = sIt.Get();
      if (compare(VN, V))
      {
        outNIt.SetCenterPixel(VN);
        V = VN;
      }
    }
    InputImagePixelType iV = mskNIt.GetCenterPixel();
    if (compare(V, iV))
    {
      outNIt.SetCenterPixel(iV);
      V = iV;
    }

    // now put indexes in the fifo
    // typename CNInputIterator::ConstIterator mIt;
    for (oLIt = oIndexList.begin(), mLIt = mIndexList.begin(); oLIt != oIndexList.end(); ++oLIt, ++mLIt)
    {
      InputImagePixelType VN = outNIt.GetPixel(*oLIt);
      InputImagePixelType iN = mskNIt.GetPixel(*mLIt);
      if (compare(V, VN) && compare(iN, VN))
      {
        IndexFifo.push(outNIt.GetIndex());
        break;
      }
    }
    progress.CompletedPixel();
  }

  // Now we want to check the full neighborhood
  setConnectivity(&outNIt, m_FullyConnected);
  setConnectivity(&mskNIt, m_FullyConnected);
  mskNIt.OverrideBoundaryCondition(&iBC);
  outNIt.OverrideBoundaryCondition(&oBC);
  oIndexList = outNIt.GetActiveIndexList();
  mIndexList = mskNIt.GetActiveIndexList();
  // now process the fifo - this fill the parts that weren't dealt
  // with by the raster and anti-raster passes
  // typename NOutputIterator::Iterator sIt;
  typename CNInputIterator::ConstIterator mIt;

  while (!IndexFifo.empty())
  {
    InputImageIndexType I = IndexFifo.front();
    IndexFifo.pop();
    // reposition the iterators
    outNIt += I - outNIt.GetIndex();
    mskNIt += I - mskNIt.GetIndex();
    InputImagePixelType V = outNIt.GetCenterPixel();
    for (oLIt = oIndexList.begin(), mLIt = mIndexList.begin(); oLIt != oIndexList.end(); ++oLIt, ++mLIt)
    {
      InputImagePixelType VN = outNIt.GetPixel(*oLIt);
      InputImagePixelType iN = mskNIt.GetPixel(*mLIt);
      // candidate for dilation via flooding
      if (compare(V, VN) && Math::NotAlmostEquals(iN, VN))
      {
        if (compare(iN, V))
        {
          // not clamped by the mask, propagate the center value
          outNIt.SetPixel(*oLIt, V);
        }
        else
        {
          // apply the clamping
          outNIt.SetPixel(*oLIt, iN);
        }
        IndexFifo.push(outNIt.GetIndex(*oLIt));
      }
    }
    progress.CompletedPixel();
  }

  if (m_UseInternalCopy)
  {
    using CropType = typename itk::CropImageFilter<InputImageType, OutputImageType>;
    auto crop = CropType::New();

    crop->SetInput(markerImageP);
    crop->SetUpperBoundaryCropSize(padSize);
    crop->SetLowerBoundaryCropSize(padSize);
    crop->GraftOutput(this->GetOutput());
    /** execute the minipipeline */
    crop->Update();

    /** graft the minipipeline output back into this filter's output */
    this->GraftOutput(crop->GetOutput());
  }
}

template <typename TInputImage, typename TOutputImage, typename TCompare>
void
ReconstructionImageFilter<TInputImage, TOutputImage, TCompare>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "FullyConnected: " << (m_FullyConnected ? "On" : "Off") << std::endl;
  os << indent << "MarkerValue: " << m_MarkerValue << std::endl;
  os << indent << "UseInternalCopy: " << m_UseInternalCopy << std::endl;
}
} // namespace itk
#endif
