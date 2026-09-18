// 补测先前判定「能造数据」的 35 个用户接口。
// precision_extra35_bench --list
// precision_extra35_bench <image.png> [runs=3] [OperatorName]
#include "itkArray.h"
#include "itkBayesianClassifierImageFilter.h"
#include "itkBayesianClassifierInitializationImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBSplineControlPointImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkComposeImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkDiffusionTensor3D.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGaussianImageSource.h"
#include "itkLandweberDeconvolutionImageFilter.h"
#include "itkGetAverageSliceImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkGradientVectorFlowImageFilter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkImplicitManifoldNormalVectorFilter.h"
#include "itkInterpolateImagePointsFilter.h"
#include "itkInverseDisplacementFieldImageFilter.h"
#include "itkJoinSeriesImageFilter.h"
#include "itkKLMRegionGrowImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkLabelMapContourOverlayImageFilter.h"
#include "itkMaskFeaturePointSelectionFilter.h"
#include "itkMaskedMovingHistogramImageFilter.h"
#include "itkMatrix.h"
#include "itkMatrixIndexSelectionImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkMetaImageIOFactory.h"
#include "itkNormalVectorDiffusionFunction.h"
#include "itkSparseImage.h"
#include "itkMRIBiasFieldCorrectionFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkMultiThreaderBase.h"
#include "itkOrientImageFilter.h"
#include "itkPNGImageIOFactory.h"
#include "itkParametricBlindLeastSquaresDeconvolutionImageFilter.h"
#include "itkPolylineMask2DImageFilter.h"
#include "itkPolylineMaskImageFilter.h"
#include "itkPolyLineParametricPath.h"
#include "itkProjectedIterativeDeconvolutionImageFilter.h"
#include "itkRankHistogram.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkRGBPixel.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkSparseFieldFourthOrderLevelSetImageFilter.h"
#include "itkSliceBySliceImageFilter.h"
#include "itkSpatialFunctionImageEvaluatorFilter.h"
#include "itkSphereSpatialFunction.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkSymmetricSecondRankTensor.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkTensorRelativeAnisotropyImageFilter.h"
#include "itkTimeVaryingVelocityFieldIntegrationImageFilter.h"
#include "itkTranslationTransform.h"
#include "itkVector.h"
#include "itkVectorConnectedComponentImageFilter.h"
#include "itkVectorExpandImageFilter.h"
#include "itkVectorImage.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkVectorThresholdSegmentationLevelSetImageFilter.h"
#include "itkVnlComplexToComplex1DFFTImageFilter.h"
#include "itkVnlComplexToComplexFFTImageFilter.h"
#include "itkVnlForward1DFFTImageFilter.h"
#include "itkVnlForwardFFTImageFilter.h"
#include "itkVoronoiSegmentationRGBImageFilter.h"

#if defined(ITK_USE_FFTWD) || defined(ITK_USE_FFTWF)
#  include "itkCurvatureRegistrationFilter.h"
#endif

#include <chrono>
#include <complex>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::steady_clock;
constexpr unsigned int Dim = 2;
using FImg = itk::Image<float, Dim>;
using DImg = itk::Image<double, Dim>;
using U8Img = itk::Image<unsigned char, Dim>;
using U16Img = itk::Image<unsigned short, Dim>;
using Img3F = itk::Image<float, 3>;
using Img3D = itk::Image<double, 3>;
using U8_3 = itk::Image<unsigned char, 3>;
using Vec2F = itk::Vector<float, 2>;
using Vec2D = itk::Vector<double, 2>;
using Vec3F = itk::Vector<float, 3>;
using VFImg = itk::Image<Vec2F, Dim>;
using VDImg = itk::Image<Vec2D, Dim>;
using VF3 = itk::Image<Vec2F, 3>;
using VD3 = itk::Image<Vec2D, 3>;
using V3ImgF = itk::Image<Vec3F, Dim>;
using CFImg = itk::Image<std::complex<float>, Dim>;
using CDImg = itk::Image<std::complex<double>, Dim>;
using RGBImg = itk::Image<itk::RGBPixel<unsigned char>, Dim>;
using Kernel = itk::FlatStructuringElement<Dim>;
using ShapeLO = itk::ShapeLabelObject<unsigned short, Dim>;
using ShapeLM = itk::LabelMap<ShapeLO>;
using DT_F = itk::DiffusionTensor3D<float>;
using DT_D = itk::DiffusionTensor3D<double>;
using DTImgF = itk::Image<DT_F, 3>;
using DTImgD = itk::Image<DT_D, 3>;
using Ten2F = itk::SymmetricSecondRankTensor<float, 2>;
using Ten2D = itk::SymmetricSecondRankTensor<double, 2>;
using Ten2ImgF = itk::Image<Ten2F, 2>;
using Ten2ImgD = itk::Image<Ten2D, 2>;
using Mat2F = itk::Matrix<float, 2, 2>;
using Mat2D = itk::Matrix<double, 2, 2>;
using MatImgF = itk::Image<Mat2F, 2>;
using MatImgD = itk::Image<Mat2D, 2>;
using V1F = itk::Vector<float, 1>;
using V1D = itk::Vector<double, 1>;
using V1ImgF = itk::Image<V1F, 2>;
using V1ImgD = itk::Image<V1D, 2>;
using Path2 = itk::PolyLineParametricPath<2>;
using Path3 = itk::PolyLineParametricPath<3>;

static std::string g_only;
static bool        g_list = false;

static bool
Want(const std::string & op)
{
  if (g_list)
  {
    std::cout << op << '\n';
    return false;
  }
  return g_only.empty() || g_only == op;
}

template <typename TImage>
static typename TImage::Pointer
Hold(TImage * p)
{
  typename TImage::Pointer out(p);
  out->DisconnectPipeline();
  return out;
}

template <typename Fn>
static double
TimeRuns(Fn fn, int runs)
{
  double t = 0.0;
  for (int i = 0; i < runs; ++i)
  {
    const auto t0 = Clock::now();
    fn();
    t += std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  }
  return t / static_cast<double>(runs);
}

static void
Emit(const std::string & module, const std::string & op, double msF, double msD)
{
  const double sp = msF > 0.0 ? (msD / msF) : 0.0;
  std::cout << std::fixed << std::setprecision(4) << module << ',' << op << ',' << msF << ',' << msD << ',' << sp
            << ",0,0" << std::endl;
}

template <typename FnF, typename FnD>
static void
BenchTime(const std::string & module, const std::string & op, FnF runF, FnD runD, int runs)
{
  if (!Want(op))
  {
    return;
  }
  std::cerr << ">> " << module << '/' << op << std::endl;
  try
  {
    runF();
    const double msF = TimeRuns(runF, runs);
    const double msD = TimeRuns(runD, runs);
    Emit(module, op, msF, msD);
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.GetDescription() << ",\n";
  }
  catch (const std::exception & e)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL," << e.what() << ",\n";
  }
  catch (...)
  {
    std::cout << module << ',' << op << ",FAIL,FAIL,FAIL,unknown,\n";
  }
}

template <typename TImage>
static typename TImage::Pointer
CropSq(const TImage * in, unsigned n)
{
  using F = itk::RegionOfInterestImageFilter<TImage, TImage>;
  auto                        f = F::New();
  typename TImage::RegionType region;
  typename TImage::IndexType  idx;
  typename TImage::SizeType   sz;
  idx.Fill(0);
  sz.Fill(n);
  region.SetIndex(idx);
  region.SetSize(sz);
  f->SetRegionOfInterest(region);
  f->SetInput(in);
  f->Update();
  return Hold(f->GetOutput());
}

template <typename TImage>
static typename TImage::Pointer
Bin(const TImage * in, typename TImage::PixelType lo = 40)
{
  using F = itk::BinaryThresholdImageFilter<TImage, TImage>;
  auto f = F::New();
  f->SetInput(in);
  f->SetLowerThreshold(lo);
  f->SetUpperThreshold(255);
  f->SetInsideValue(1);
  f->SetOutsideValue(0);
  f->Update();
  return Hold(f->GetOutput());
}

template <typename T2, typename T3>
static typename T3::Pointer
Stack3(const T2 * in2, unsigned nz)
{
  auto                     o = T3::New();
  typename T3::IndexType   idx;
  typename T3::SizeType    sz;
  typename T3::RegionType  region;
  typename T3::SpacingType sp;
  idx.Fill(0);
  const auto s2 = in2->GetLargestPossibleRegion().GetSize();
  sz[0] = s2[0];
  sz[1] = s2[1];
  sz[2] = nz;
  region.SetIndex(idx);
  region.SetSize(sz);
  o->SetRegions(region);
  const auto sp2 = in2->GetSpacing();
  sp[0] = sp2[0];
  sp[1] = sp2[1];
  sp[2] = 1.0;
  o->SetSpacing(sp);
  o->Allocate();
  itk::ImageRegionIterator<T3> it(o, region);
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto               i = it.GetIndex();
    typename T2::IndexType   j;
    j[0] = i[0];
    j[1] = i[1];
    const double v = static_cast<double>(in2->GetPixel(j));
    it.Set(static_cast<typename T3::PixelType>(v * (1.0 + 0.02 * static_cast<double>(i[2]))));
  }
  return o;
}

template <typename TField>
static typename TField::Pointer
MakeVaryingDisp(unsigned n, double sx = 1.0, double sy = 1.0)
{
  auto                        o = TField::New();
  typename TField::RegionType region;
  typename TField::IndexType  idx;
  typename TField::SizeType   sz;
  idx.Fill(0);
  sz.Fill(n);
  region.SetIndex(idx);
  region.SetSize(sz);
  o->SetRegions(region);
  typename TField::SpacingType sp;
  sp.Fill(1.0);
  o->SetSpacing(sp);
  o->Allocate();
  itk::ImageRegionIterator<TField> it(o, region);
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const auto                   i = it.GetIndex();
    typename TField::PixelType   v;
    v[0] = static_cast<typename TField::PixelType::ValueType>(sx * 0.02 * static_cast<double>(i[0]));
    v[1] = static_cast<typename TField::PixelType::ValueType>(sy * 0.01 * static_cast<double>(i[1]));
    it.Set(v);
  }
  return o;
}

template <typename TDTImg>
static typename TDTImg::Pointer
MakeTensorVol(unsigned n, unsigned nz, double diag)
{
  auto                        o = TDTImg::New();
  typename TDTImg::IndexType  idx;
  typename TDTImg::SizeType   sz;
  typename TDTImg::RegionType region;
  idx.Fill(0);
  sz[0] = n;
  sz[1] = n;
  sz[2] = nz;
  region.SetIndex(idx);
  region.SetSize(sz);
  o->SetRegions(region);
  o->Allocate();
  typename TDTImg::PixelType t;
  t.Fill(0);
  t(0, 0) = static_cast<typename TDTImg::PixelType::ComponentType>(diag);
  t(1, 1) = static_cast<typename TDTImg::PixelType::ComponentType>(diag);
  t(2, 2) = static_cast<typename TDTImg::PixelType::ComponentType>(diag * 0.5);
  o->FillBuffer(t);
  return o;
}

static Path2::Pointer
MakeSquarePath(double x0, double y0, double x1, double y1)
{
  auto p = Path2::New();
  p->Initialize();
  Path2::ContinuousIndexType c;
  c[0] = x0;
  c[1] = y0;
  p->AddVertex(c);
  c[0] = x1;
  c[1] = y0;
  p->AddVertex(c);
  c[0] = x1;
  c[1] = y1;
  p->AddVertex(c);
  c[0] = x0;
  c[1] = y1;
  p->AddVertex(c);
  c[0] = x0;
  c[1] = y0;
  p->AddVertex(c);
  return p;
}

int
main(int argc, char ** argv)
{
  if (argc >= 2 && std::string(argv[1]) == "--list")
  {
    g_list = true;
  }
  else if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " --list | <image.png> [runs=3] [OperatorName]\n";
    return 1;
  }
  itk::PNGImageIOFactory::RegisterOneFactory();
  itk::MetaImageIOFactory::RegisterOneFactory();
  const int runs = (!g_list && argc >= 3) ? std::stoi(argv[2]) : 3;
  if (!g_list && argc >= 4)
  {
    g_only = argv[3];
  }

  FImg::Pointer    input, smallF, binF, smallBinF;
  DImg::Pointer    inputD, smallD, binD, smallBinD;
  Img3F::Pointer   volF, vol32F;
  Img3D::Pointer   volD, vol32D;
  U8_3::Pointer    mask3;
  VFImg::Pointer   dispF;
  VDImg::Pointer   dispD;
  VF3::Pointer     tvfF;
  VD3::Pointer     tvfD;
  DTImgF::Pointer  tenF;
  DTImgD::Pointer  tenD;
  Ten2ImgF::Pointer t2F;
  Ten2ImgD::Pointer t2D;
  MatImgF::Pointer matF;
  MatImgD::Pointer matD;
  V1ImgF::Pointer  v1F;
  V1ImgD::Pointer  v1D;
  V3ImgF::Pointer  rgbVec;
  CFImg::Pointer   specF;
  CDImg::Pointer   specD;
  ShapeLM::Pointer shapeMap;
  U16Img::Pointer  lab16;
  U8Img::Pointer   mask8;
  Path2::Pointer   poly;
  Img3F::Pointer   kerF;
  Img3D::Pointer   kerD;
  FImg::Pointer    initLSF;
  DImg::Pointer    initLSD;
  FImg::Pointer    coordXF, coordYF;
  DImg::Pointer    coordXD, coordYD;
  VFImg::Pointer   latticeF;
  VDImg::Pointer   latticeD;

  if (!g_list)
  {
    using Reader = itk::ImageFileReader<FImg>;
    auto reader = Reader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    input = Hold(reader->GetOutput());
    using Cast = itk::CastImageFilter<FImg, DImg>;
    auto c = Cast::New();
    c->SetInput(input);
    c->Update();
    inputD = Hold(c->GetOutput());
    smallF = CropSq(input.GetPointer(), 64);
    smallD = CropSq(inputD.GetPointer(), 64);
    binF = Bin(input.GetPointer());
    binD = Bin(inputD.GetPointer());
    smallBinF = Bin(smallF.GetPointer());
    smallBinD = Bin(smallD.GetPointer());
    volF = Stack3<FImg, Img3F>(CropSq(input.GetPointer(), 32).GetPointer(), 8);
    volD = Stack3<DImg, Img3D>(CropSq(inputD.GetPointer(), 32).GetPointer(), 8);
    vol32F = Stack3<FImg, Img3F>(CropSq(input.GetPointer(), 32).GetPointer(), 16);
    vol32D = Stack3<DImg, Img3D>(CropSq(inputD.GetPointer(), 32).GetPointer(), 16);
    mask3 = U8_3::New();
    mask3->CopyInformation(vol32F);
    mask3->SetRegions(vol32F->GetLargestPossibleRegion());
    mask3->Allocate();
    mask3->FillBuffer(1);
    dispF = MakeVaryingDisp<VFImg>(32);
    dispD = MakeVaryingDisp<VDImg>(32);
    {
      tvfF = VF3::New();
      VF3::IndexType i0;
      i0.Fill(0);
      VF3::SizeType s;
      s[0] = 32;
      s[1] = 32;
      s[2] = 4;
      VF3::RegionType r;
      r.SetIndex(i0);
      r.SetSize(s);
      tvfF->SetRegions(r);
      tvfF->Allocate();
      itk::ImageRegionIterator<VF3> it(tvfF, r);
      for (it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
        const auto ix = it.GetIndex();
        Vec2F      v;
        v[0] = 0.02f * static_cast<float>(ix[0]);
        v[1] = 0.01f * static_cast<float>(ix[1]);
        it.Set(v);
      }
      tvfD = VD3::New();
      tvfD->SetRegions(r);
      tvfD->Allocate();
      itk::ImageRegionIterator<VD3> itd(tvfD, r);
      for (itd.GoToBegin(); !itd.IsAtEnd(); ++itd)
      {
        const auto ix = itd.GetIndex();
        Vec2D      v;
        v[0] = 0.02 * static_cast<double>(ix[0]);
        v[1] = 0.01 * static_cast<double>(ix[1]);
        itd.Set(v);
      }
    }
    tenF = MakeTensorVol<DTImgF>(32, 8, 0.001);
    tenD = MakeTensorVol<DTImgD>(32, 8, 0.001);
    t2F = Ten2ImgF::New();
    t2F->SetRegions(smallF->GetLargestPossibleRegion());
    t2F->CopyInformation(smallF);
    t2F->Allocate();
    {
      Ten2F t;
      t.Fill(0);
      t(0, 0) = 1.f;
      t(1, 1) = 0.5f;
      t2F->FillBuffer(t);
    }
    t2D = Ten2ImgD::New();
    t2D->SetRegions(smallD->GetLargestPossibleRegion());
    t2D->CopyInformation(smallD);
    t2D->Allocate();
    {
      Ten2D t;
      t.Fill(0);
      t(0, 0) = 1.0;
      t(1, 1) = 0.5;
      t2D->FillBuffer(t);
    }
    matF = MatImgF::New();
    matF->SetRegions(smallF->GetLargestPossibleRegion());
    matF->CopyInformation(smallF);
    matF->Allocate();
    {
      Mat2F m;
      m.Fill(0);
      m(0, 0) = 2.f;
      m(1, 1) = 3.f;
      matF->FillBuffer(m);
    }
    matD = MatImgD::New();
    matD->SetRegions(smallD->GetLargestPossibleRegion());
    matD->CopyInformation(smallD);
    matD->Allocate();
    {
      Mat2D m;
      m.Fill(0);
      m(0, 0) = 2.0;
      m(1, 1) = 3.0;
      matD->FillBuffer(m);
    }
    v1F = V1ImgF::New();
    v1F->SetRegions(smallF->GetLargestPossibleRegion());
    v1F->CopyInformation(smallF);
    v1F->Allocate();
    v1D = V1ImgD::New();
    v1D->SetRegions(smallD->GetLargestPossibleRegion());
    v1D->CopyInformation(smallD);
    v1D->Allocate();
    {
      itk::ImageRegionConstIterator<FImg> a(smallF, smallF->GetLargestPossibleRegion());
      itk::ImageRegionIterator<V1ImgF>    b(v1F, v1F->GetLargestPossibleRegion());
      for (a.GoToBegin(), b.GoToBegin(); !a.IsAtEnd(); ++a, ++b)
      {
        V1F p;
        p[0] = a.Get();
        b.Set(p);
      }
      itk::ImageRegionConstIterator<DImg> c2(smallD, smallD->GetLargestPossibleRegion());
      itk::ImageRegionIterator<V1ImgD>    d2(v1D, v1D->GetLargestPossibleRegion());
      for (c2.GoToBegin(), d2.GoToBegin(); !c2.IsAtEnd(); ++c2, ++d2)
      {
        V1D p;
        p[0] = c2.Get();
        d2.Set(p);
      }
    }
    rgbVec = V3ImgF::New();
    rgbVec->SetRegions(smallF->GetLargestPossibleRegion());
    rgbVec->CopyInformation(smallF);
    rgbVec->Allocate();
    {
      itk::ImageRegionConstIterator<FImg> a(smallF, smallF->GetLargestPossibleRegion());
      itk::ImageRegionIterator<V3ImgF>    b(rgbVec, rgbVec->GetLargestPossibleRegion());
      for (a.GoToBegin(), b.GoToBegin(); !a.IsAtEnd(); ++a, ++b)
      {
        Vec3F p;
        p[0] = a.Get();
        p[1] = a.Get();
        p[2] = a.Get() * 0.8f;
        b.Set(p);
      }
    }
    {
      auto fwd = itk::VnlForwardFFTImageFilter<FImg, CFImg>::New();
      fwd->SetInput(smallF);
      fwd->Update();
      specF = Hold(fwd->GetOutput());
      auto fwdD = itk::VnlForwardFFTImageFilter<DImg, CDImg>::New();
      fwdD->SetInput(smallD);
      fwdD->Update();
      specD = Hold(fwdD->GetOutput());
    }
    {
      using CC = itk::ConnectedComponentImageFilter<FImg, U16Img>;
      auto cc = CC::New();
      cc->SetInput(smallBinF);
      cc->Update();
      lab16 = Hold(cc->GetOutput());
      using ToShape = itk::LabelImageToShapeLabelMapFilter<U16Img, ShapeLM>;
      auto ts = ToShape::New();
      ts->SetInput(lab16);
      ts->Update();
      shapeMap = ts->GetOutput();
      shapeMap->DisconnectPipeline();
    }
    mask8 = U8Img::New();
    mask8->SetRegions(smallF->GetLargestPossibleRegion());
    mask8->CopyInformation(smallF);
    mask8->Allocate();
    mask8->FillBuffer(1);
    poly = MakeSquarePath(8, 8, 56, 56);
    {
      using G = itk::GaussianImageSource<Img3F>;
      auto g = G::New();
      Img3F::SizeType sz;
      sz.Fill(7);
      g->SetSize(sz);
      g->SetSpacing(volF->GetSpacing());
      g->SetNormalized(true);
      g->Update();
      kerF = Hold(g->GetOutput());
      using GD = itk::GaussianImageSource<Img3D>;
      auto gd = GD::New();
      Img3D::SizeType szd;
      szd.Fill(7);
      gd->SetSize(szd);
      gd->SetSpacing(volD->GetSpacing());
      gd->SetNormalized(true);
      gd->Update();
      kerD = Hold(gd->GetOutput());
    }
    {
      auto dist = itk::SignedMaurerDistanceMapImageFilter<FImg, FImg>::New();
      dist->SetInput(smallBinF);
      dist->SetInsideIsPositive(false);
      dist->SetUseImageSpacing(false);
      dist->Update();
      initLSF = Hold(dist->GetOutput());
      auto distD = itk::SignedMaurerDistanceMapImageFilter<DImg, DImg>::New();
      distD->SetInput(smallBinD);
      distD->SetInsideIsPositive(false);
      distD->SetUseImageSpacing(false);
      distD->Update();
      initLSD = Hold(distD->GetOutput());
    }
    coordXF = FImg::New();
    coordYF = FImg::New();
    coordXF->SetRegions(smallF->GetLargestPossibleRegion());
    coordYF->SetRegions(smallF->GetLargestPossibleRegion());
    coordXF->CopyInformation(smallF);
    coordYF->CopyInformation(smallF);
    coordXF->Allocate();
    coordYF->Allocate();
    coordXD = DImg::New();
    coordYD = DImg::New();
    coordXD->SetRegions(smallD->GetLargestPossibleRegion());
    coordYD->SetRegions(smallD->GetLargestPossibleRegion());
    coordXD->CopyInformation(smallD);
    coordYD->CopyInformation(smallD);
    coordXD->Allocate();
    coordYD->Allocate();
    {
      itk::ImageRegionIterator<FImg> x(coordXF, coordXF->GetLargestPossibleRegion());
      itk::ImageRegionIterator<FImg> y(coordYF, coordYF->GetLargestPossibleRegion());
      for (x.GoToBegin(), y.GoToBegin(); !x.IsAtEnd(); ++x, ++y)
      {
        const auto i = x.GetIndex();
        x.Set(static_cast<float>(i[0]));
        y.Set(static_cast<float>(i[1]));
      }
      itk::ImageRegionIterator<DImg> xd(coordXD, coordXD->GetLargestPossibleRegion());
      itk::ImageRegionIterator<DImg> yd(coordYD, coordYD->GetLargestPossibleRegion());
      for (xd.GoToBegin(), yd.GoToBegin(); !xd.IsAtEnd(); ++xd, ++yd)
      {
        const auto i = xd.GetIndex();
        xd.Set(static_cast<double>(i[0]));
        yd.Set(static_cast<double>(i[1]));
      }
    }
    latticeF = MakeVaryingDisp<VFImg>(8, 4.0, 4.0);
    latticeD = MakeVaryingDisp<VDImg>(8, 4.0, 4.0);
  }

  BenchTime(
    "BiasCorrection",
    "MRIBiasFieldCorrectionFilter",
    [&]() {
      using F = itk::MRIBiasFieldCorrectionFilter<Img3F, Img3F, U8_3>;
      auto f = F::New();
      f->SetInput(volF);
      f->SetUsingInterSliceIntensityCorrection(false);
      f->SetUsingSlabIdentification(false);
      f->SetBiasFieldMultiplicative(false);
      f->SetBiasFieldDegree(1);
      f->SetNumberOfLevels(1);
      f->SetVolumeCorrectionMaximumIteration(2);
      itk::Array<double> means(2), sigmas(2);
      means[0] = 40;
      means[1] = 120;
      sigmas[0] = 12;
      sigmas[1] = 20;
      f->SetTissueClassStatistics(means, sigmas);
      f->Update();
    },
    [&]() {
      using F = itk::MRIBiasFieldCorrectionFilter<Img3D, Img3D, U8_3>;
      auto f = F::New();
      f->SetInput(volD);
      f->SetUsingInterSliceIntensityCorrection(false);
      f->SetUsingSlabIdentification(false);
      f->SetBiasFieldMultiplicative(false);
      f->SetBiasFieldDegree(1);
      f->SetNumberOfLevels(1);
      f->SetVolumeCorrectionMaximumIteration(2);
      itk::Array<double> means(2), sigmas(2);
      means[0] = 40;
      means[1] = 120;
      sigmas[0] = 12;
      sigmas[1] = 20;
      f->SetTissueClassStatistics(means, sigmas);
      f->Update();
    },
    runs);

  BenchTime(
    "DiffusionTensorImage",
    "TensorFractionalAnisotropyImageFilter",
    [&]() {
      auto f = itk::TensorFractionalAnisotropyImageFilter<DTImgF, Img3F>::New();
      f->SetInput(tenF);
      f->Update();
    },
    [&]() {
      auto f = itk::TensorFractionalAnisotropyImageFilter<DTImgD, Img3D>::New();
      f->SetInput(tenD);
      f->Update();
    },
    runs);
  BenchTime(
    "DiffusionTensorImage",
    "TensorRelativeAnisotropyImageFilter",
    [&]() {
      auto f = itk::TensorRelativeAnisotropyImageFilter<DTImgF, Img3F>::New();
      f->SetInput(tenF);
      f->Update();
    },
    [&]() {
      auto f = itk::TensorRelativeAnisotropyImageFilter<DTImgD, Img3D>::New();
      f->SetInput(tenD);
      f->Update();
    },
    runs);
  BenchTime(
    "DiffusionTensorImage",
    "DiffusionTensor3DReconstructionImageFilter",
    [&]() {
      using Rec = itk::DiffusionTensor3DReconstructionImageFilter<float, float, float>;
      auto f = Rec::New();
      f->SetNumberOfWorkUnits(1);
      f->SetReferenceImage(volF);
      f->SetThreshold(1.f);
      f->SetBValue(1000);
      const double dirs[6][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
                                  { 0.7071, 0.7071, 0 }, { 0.7071, 0, 0.7071 }, { 0, 0.7071, 0.7071 } };
      for (int k = 0; k < 6; ++k)
      {
        Rec::GradientDirectionType g;
        g[0] = dirs[k][0];
        g[1] = dirs[k][1];
        g[2] = dirs[k][2];
        f->AddGradientImage(g, volF);
      }
      f->Update();
    },
    [&]() {
      using Rec = itk::DiffusionTensor3DReconstructionImageFilter<double, double, double>;
      auto f = Rec::New();
      f->SetNumberOfWorkUnits(1);
      f->SetReferenceImage(volD);
      f->SetThreshold(1.0);
      f->SetBValue(1000);
      const double dirs[6][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
                                  { 0.7071, 0.7071, 0 }, { 0.7071, 0, 0.7071 }, { 0, 0.7071, 0.7071 } };
      for (int k = 0; k < 6; ++k)
      {
        Rec::GradientDirectionType g;
        g[0] = dirs[k][0];
        g[1] = dirs[k][1];
        g[2] = dirs[k][2];
        f->AddGradientImage(g, volD);
      }
      f->Update();
    },
    runs);

  BenchTime(
    "ImageFeature",
    "Hessian3DToVesselnessMeasureImageFilter",
    [&]() {
      auto h = itk::HessianRecursiveGaussianImageFilter<Img3F>::New();
      h->SetInput(volF);
      h->SetSigma(1.0);
      h->Update();
      auto f = itk::Hessian3DToVesselnessMeasureImageFilter<float>::New();
      f->SetInput(h->GetOutput());
      f->Update();
    },
    [&]() {
      auto h = itk::HessianRecursiveGaussianImageFilter<Img3D>::New();
      h->SetInput(volD);
      h->SetSigma(1.0);
      h->Update();
      auto f = itk::Hessian3DToVesselnessMeasureImageFilter<double>::New();
      f->SetInput(h->GetOutput());
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFeature",
    "MultiScaleHessianBasedMeasureImageFilter",
    [&]() {
      using Hess = itk::HessianRecursiveGaussianImageFilter<Img3F>;
      using Obj = itk::HessianToObjectnessMeasureImageFilter<Hess::OutputImageType, Img3F>;
      using Multi = itk::MultiScaleHessianBasedMeasureImageFilter<Img3F, Hess::OutputImageType, Img3F>;
      auto obj = Obj::New();
      obj->SetObjectDimension(1);
      auto f = Multi::New();
      f->SetInput(volF);
      f->SetHessianToMeasureFilter(obj);
      f->SetSigmaMinimum(1.0);
      f->SetSigmaMaximum(2.0);
      f->SetNumberOfSigmaSteps(2);
      f->Update();
    },
    [&]() {
      using Hess = itk::HessianRecursiveGaussianImageFilter<Img3D>;
      using Obj = itk::HessianToObjectnessMeasureImageFilter<Hess::OutputImageType, Img3D>;
      using Multi = itk::MultiScaleHessianBasedMeasureImageFilter<Img3D, Hess::OutputImageType, Img3D>;
      auto obj = Obj::New();
      obj->SetObjectDimension(1);
      auto f = Multi::New();
      f->SetInput(volD);
      f->SetHessianToMeasureFilter(obj);
      f->SetSigmaMinimum(1.0);
      f->SetSigmaMaximum(2.0);
      f->SetNumberOfSigmaSteps(2);
      f->Update();
    },
    runs);

  BenchTime(
    "Deconvolution",
    "ParametricBlindLeastSquaresDeconvolutionImageFilter",
    [&]() {
      using Ker = itk::GaussianImageSource<FImg>;
      using F = itk::ParametricBlindLeastSquaresDeconvolutionImageFilter<FImg, Ker, FImg>;
      auto src = Ker::New();
      FImg::SizeType ksz;
      ksz.Fill(7);
      src->SetSize(ksz);
      src->SetNormalized(true);
      auto f = F::New();
      f->SetInput(CropSq(smallF.GetPointer(), 32));
      f->SetKernelSource(src);
      f->SetNumberOfIterations(1);
      f->SetAlpha(0.1);
      f->Update();
    },
    [&]() {
      using Ker = itk::GaussianImageSource<DImg>;
      using F = itk::ParametricBlindLeastSquaresDeconvolutionImageFilter<DImg, Ker, DImg>;
      auto src = Ker::New();
      DImg::SizeType ksz;
      ksz.Fill(7);
      src->SetSize(ksz);
      src->SetNormalized(true);
      auto f = F::New();
      f->SetInput(CropSq(smallD.GetPointer(), 32));
      f->SetKernelSource(src);
      f->SetNumberOfIterations(1);
      f->SetAlpha(0.1);
      f->Update();
    },
    runs);
  BenchTime(
    "Deconvolution",
    "ProjectedIterativeDeconvolutionImageFilter",
    [&]() {
      using Base = itk::LandweberDeconvolutionImageFilter<FImg>;
      using F = itk::ProjectedIterativeDeconvolutionImageFilter<Base>;
      auto f = F::New();
      f->SetInput(CropSq(smallF.GetPointer(), 32));
      f->SetKernelImage(CropSq(smallF.GetPointer(), 7));
      f->SetNumberOfIterations(1);
      f->Update();
    },
    [&]() {
      using Base = itk::LandweberDeconvolutionImageFilter<DImg>;
      using F = itk::ProjectedIterativeDeconvolutionImageFilter<Base>;
      auto f = F::New();
      f->SetInput(CropSq(smallD.GetPointer(), 32));
      f->SetKernelImage(CropSq(smallD.GetPointer(), 7));
      f->SetNumberOfIterations(1);
      f->Update();
    },
    runs);

  if (Want("CurvatureRegistrationFilter"))
  {
    std::cerr << ">> PDEDeformable/CurvatureRegistrationFilter" << std::endl;
#if defined(ITK_USE_FFTWD) || defined(ITK_USE_FFTWF)
    try
    {
      auto run = [&](auto * fix, auto * mov) {
        using Img = typename std::remove_pointer<decltype(fix)>::type;
        using Vec = itk::Vector<typename Img::PixelType, 2>;
        using Field = itk::Image<Vec, 2>;
        auto f = itk::CurvatureRegistrationFilter<Img, Img, Field>::New();
        f->SetFixedImage(fix);
        f->SetMovingImage(mov);
        f->SetNumberOfIterations(1);
        f->Update();
      };
      run(smallF.GetPointer(), smallF.GetPointer());
      const double msF = TimeRuns([&]() { run(smallF.GetPointer(), smallF.GetPointer()); }, runs);
      const double msD = TimeRuns([&]() { run(smallD.GetPointer(), smallD.GetPointer()); }, runs);
      Emit("PDEDeformable", "CurvatureRegistrationFilter", msF, msD);
    }
    catch (const itk::ExceptionObject & e)
    {
      std::cout << "PDEDeformable,CurvatureRegistrationFilter,FAIL,FAIL,FAIL," << e.GetDescription() << ",\n";
    }
#else
    std::cout << "PDEDeformable,CurvatureRegistrationFilter,FAIL,FAIL,FAIL,needs_FFTW,\n";
#endif
  }

  BenchTime(
    "ImageFeature",
    "GradientVectorFlowImageFilter",
    [&]() {
      auto g = itk::GradientImageFilter<FImg, float, float>::New();
      g->SetInput(smallF);
      g->Update();
      using GImg = itk::Image<itk::CovariantVector<float, 2>, 2>;
      auto f = itk::GradientVectorFlowImageFilter<GImg, GImg>::New();
      f->SetInput(g->GetOutput());
      f->SetIterationNum(3);
      f->SetNoiseLevel(200);
      f->SetTimeStep(0.001);
      f->Update();
    },
    [&]() {
      auto g = itk::GradientImageFilter<DImg, double, double>::New();
      g->SetInput(smallD);
      g->Update();
      using GImg = itk::Image<itk::CovariantVector<double, 2>, 2>;
      auto f = itk::GradientVectorFlowImageFilter<GImg, GImg>::New();
      f->SetInput(g->GetOutput());
      f->SetIterationNum(3);
      f->SetNoiseLevel(200);
      f->SetTimeStep(0.001);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageFeature",
    "MaskFeaturePointSelectionFilter",
    [&]() {
      auto f = itk::MaskFeaturePointSelectionFilter<Img3F, U8_3>::New();
      f->SetInput(vol32F);
      f->SetMaskImage(mask3);
      f->SetSelectFraction(0.02);
      f->ComputeStructureTensorsOff();
      typename itk::MaskFeaturePointSelectionFilter<Img3F, U8_3>::SizeType br;
      br.Fill(1);
      f->SetBlockRadius(br);
      f->Update();
    },
    [&]() {
      auto f = itk::MaskFeaturePointSelectionFilter<Img3D, U8_3>::New();
      f->SetInput(vol32D);
      f->SetMaskImage(mask3);
      f->SetSelectFraction(0.02);
      f->ComputeStructureTensorsOff();
      typename itk::MaskFeaturePointSelectionFilter<Img3D, U8_3>::SizeType br;
      br.Fill(1);
      f->SetBlockRadius(br);
      f->Update();
    },
    runs);

  BenchTime(
    "FFT",
    "VnlComplexToComplexFFTImageFilter",
    [&]() {
      auto f = itk::VnlComplexToComplexFFTImageFilter<CFImg>::New();
      f->SetInput(specF);
      f->SetTransformDirection(itk::VnlComplexToComplexFFTImageFilter<CFImg>::INVERSE);
      f->Update();
    },
    [&]() {
      auto f = itk::VnlComplexToComplexFFTImageFilter<CDImg>::New();
      f->SetInput(specD);
      f->SetTransformDirection(itk::VnlComplexToComplexFFTImageFilter<CDImg>::INVERSE);
      f->Update();
    },
    runs);
  BenchTime(
    "FFT",
    "VnlComplexToComplex1DFFTImageFilter",
    [&]() {
      auto f = itk::VnlComplexToComplex1DFFTImageFilter<CFImg>::New();
      f->SetInput(specF);
      f->SetDirection(0);
      f->SetTransformDirection(itk::VnlComplexToComplex1DFFTImageFilter<CFImg>::INVERSE);
      f->Update();
    },
    [&]() {
      auto f = itk::VnlComplexToComplex1DFFTImageFilter<CDImg>::New();
      f->SetInput(specD);
      f->SetDirection(0);
      f->SetTransformDirection(itk::VnlComplexToComplex1DFFTImageFilter<CDImg>::INVERSE);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageGrid",
    "BSplineControlPointImageFilter",
    [&]() {
      using F = itk::BSplineControlPointImageFilter<VFImg, VFImg>;
      auto f = F::New();
      f->SetInput(latticeF);
      f->SetSplineOrder(3);
      typename F::SizeType sz;
      sz.Fill(32);
      f->SetSize(sz);
      typename F::SpacingType sp;
      sp.Fill(1.0);
      f->SetSpacing(sp);
      typename F::OriginType or0;
      or0.Fill(0.0);
      f->SetOrigin(or0);
      f->Update();
    },
    [&]() {
      using F = itk::BSplineControlPointImageFilter<VDImg, VDImg>;
      auto f = F::New();
      f->SetInput(latticeD);
      f->SetSplineOrder(3);
      typename F::SizeType sz;
      sz.Fill(32);
      f->SetSize(sz);
      typename F::SpacingType sp;
      sp.Fill(1.0);
      f->SetSpacing(sp);
      typename F::OriginType or0;
      or0.Fill(0.0);
      f->SetOrigin(or0);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "InterpolateImagePointsFilter",
    [&]() {
      auto f = itk::InterpolateImagePointsFilter<FImg, FImg>::New();
      f->SetInputImage(smallF);
      f->SetInterpolationCoordinate(coordXF, 0);
      f->SetInterpolationCoordinate(coordYF, 1);
      f->Update();
    },
    [&]() {
      auto f = itk::InterpolateImagePointsFilter<DImg, DImg>::New();
      f->SetInputImage(smallD);
      f->SetInterpolationCoordinate(coordXD, 0);
      f->SetInterpolationCoordinate(coordYD, 1);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "OrientImageFilter",
    [&]() {
      auto f = itk::OrientImageFilter<Img3F, Img3F>::New();
      f->SetInput(volF);
      f->UseImageDirectionOn();
      f->SetDesiredCoordinateOrientationToAxial();
      f->Update();
    },
    [&]() {
      auto f = itk::OrientImageFilter<Img3D, Img3D>::New();
      f->SetInput(volD);
      f->UseImageDirectionOn();
      f->SetDesiredCoordinateOrientationToAxial();
      f->Update();
    },
    runs);
  BenchTime(
    "ImageGrid",
    "SliceBySliceImageFilter",
    [&]() {
      using Inner = itk::MedianImageFilter<FImg, FImg>;
      using SBS = itk::SliceBySliceImageFilter<Img3F, Img3F, Inner>;
      auto inner = Inner::New();
      inner->SetRadius(1);
      auto f = SBS::New();
      f->SetInput(volF);
      f->SetDimension(2);
      f->SetFilter(inner);
      f->Update();
    },
    [&]() {
      using Inner = itk::MedianImageFilter<DImg, DImg>;
      using SBS = itk::SliceBySliceImageFilter<Img3D, Img3D, Inner>;
      auto inner = Inner::New();
      inner->SetRadius(1);
      auto f = SBS::New();
      f->SetInput(volD);
      f->SetDimension(2);
      f->SetFilter(inner);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageIntensity",
    "PolylineMask2DImageFilter",
    [&]() {
      auto f = itk::PolylineMask2DImageFilter<FImg, Path2, FImg>::New();
      f->SetInput1(smallF);
      f->SetInput2(poly);
      f->Update();
    },
    [&]() {
      auto f = itk::PolylineMask2DImageFilter<DImg, Path2, DImg>::New();
      f->SetInput1(smallD);
      f->SetInput2(poly);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "PolylineMaskImageFilter",
    [&]() {
      auto path = MakeSquarePath(8, 4, 24, 28);
      using Vec = itk::Vector<double, 3>;
      using F = itk::PolylineMaskImageFilter<Img3F, Path2, Vec, Img3F>;
      auto f = F::New();
      f->SetInput1(volF);
      f->SetInput2(path);
      Vec view;
      view[0] = 0;
      view[1] = 0;
      view[2] = -1;
      Vec up;
      up[0] = 1;
      up[1] = 0;
      up[2] = 0;
      f->SetViewVector(view);
      f->SetUpVector(up);
      typename F::PointType cam;
      cam[0] = 16;
      cam[1] = 16;
      cam[2] = 40;
      f->SetCameraCenterPoint(cam);
      f->SetFocalDistance(30);
      typename F::ProjPlanePointType fp;
      fp[0] = 16;
      fp[1] = 16;
      f->SetFocalPoint(fp);
      f->Update();
    },
    [&]() {
      auto path = MakeSquarePath(8, 4, 24, 28);
      using Vec = itk::Vector<double, 3>;
      using F = itk::PolylineMaskImageFilter<Img3D, Path2, Vec, Img3D>;
      auto f = F::New();
      f->SetInput1(volD);
      f->SetInput2(path);
      Vec view;
      view[0] = 0;
      view[1] = 0;
      view[2] = -1;
      Vec up;
      up[0] = 1;
      up[1] = 0;
      up[2] = 0;
      f->SetViewVector(view);
      f->SetUpVector(up);
      typename F::PointType cam;
      cam[0] = 16;
      cam[1] = 16;
      cam[2] = 40;
      f->SetCameraCenterPoint(cam);
      f->SetFocalDistance(30);
      typename F::ProjPlanePointType fp;
      fp[0] = 16;
      fp[1] = 16;
      f->SetFocalPoint(fp);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "VectorExpandImageFilter",
    [&]() {
      auto f = itk::VectorExpandImageFilter<VFImg, VFImg>::New();
      f->SetInput(dispF);
      f->SetExpandFactors(2);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorExpandImageFilter<VDImg, VDImg>::New();
      f->SetInput(dispD);
      f->SetExpandFactors(2);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "SymmetricEigenAnalysisImageFilter",
    [&]() {
      using Out = itk::Image<itk::FixedArray<float, 2>, 2>;
      auto f = itk::SymmetricEigenAnalysisImageFilter<Ten2ImgF, Out>::New();
      f->SetInput(t2F);
      f->SetDimension(2);
      f->Update();
    },
    [&]() {
      using Out = itk::Image<itk::FixedArray<double, 2>, 2>;
      auto f = itk::SymmetricEigenAnalysisImageFilter<Ten2ImgD, Out>::New();
      f->SetInput(t2D);
      f->SetDimension(2);
      f->Update();
    },
    runs);
  BenchTime(
    "ImageIntensity",
    "MatrixIndexSelectionImageFilter",
    [&]() {
      using Min = itk::Image<itk::Matrix<unsigned short, 2, 2>, 2>;
      using Mout = itk::Image<unsigned char, 2>;
      auto in = Min::New();
      in->SetRegions(smallF->GetLargestPossibleRegion());
      in->Allocate();
      itk::Matrix<unsigned short, 2, 2> m;
      m.Fill(0);
      m(0, 0) = 9;
      in->FillBuffer(m);
      auto f = itk::MatrixIndexSelectionImageFilter<Min, Mout>::New();
      f->SetInput(in);
      f->SetIndices(0, 0);
      f->Update();
    },
    [&]() {
      using Min = itk::Image<itk::Matrix<unsigned short, 2, 2>, 2>;
      using Mout = itk::Image<unsigned char, 2>;
      auto in = Min::New();
      in->SetRegions(smallD->GetLargestPossibleRegion());
      in->Allocate();
      itk::Matrix<unsigned short, 2, 2> m;
      m.Fill(0);
      m(0, 0) = 9;
      in->FillBuffer(m);
      auto f = itk::MatrixIndexSelectionImageFilter<Min, Mout>::New();
      f->SetInput(in);
      f->SetIndices(0, 0);
      f->Update();
    },
    runs);

  BenchTime(
    "SpatialFunction",
    "SpatialFunctionImageEvaluatorFilter",
    [&]() {
      using Sphere = itk::SphereSpatialFunction<2>;
      auto s = Sphere::New();
      s->SetRadius(16);
      Sphere::InputType c;
      c.Fill(32);
      s->SetCenter(c);
      auto f = itk::SpatialFunctionImageEvaluatorFilter<Sphere, FImg, FImg>::New();
      f->SetFunction(s);
      f->SetInput(smallF);
      f->Update();
    },
    [&]() {
      using Sphere = itk::SphereSpatialFunction<2>;
      auto s = Sphere::New();
      s->SetRadius(16);
      Sphere::InputType c;
      c.Fill(32);
      s->SetCenter(c);
      auto f = itk::SpatialFunctionImageEvaluatorFilter<Sphere, DImg, DImg>::New();
      f->SetFunction(s);
      f->SetInput(smallD);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageStatistics",
    "GetAverageSliceImageFilter",
    [&]() {
      auto f = itk::GetAverageSliceImageFilter<Img3F, Img3F>::New();
      f->SetInput(volF);
      f->SetAveragedOutDimension(2);
      f->Update();
    },
    [&]() {
      auto f = itk::GetAverageSliceImageFilter<Img3D, Img3D>::New();
      f->SetInput(volD);
      f->SetAveragedOutDimension(2);
      f->Update();
    },
    runs);

  BenchTime(
    "DisplacementField",
    "InverseDisplacementFieldImageFilter",
    [&]() {
      auto f = itk::InverseDisplacementFieldImageFilter<VFImg, VFImg>::New();
      auto in = MakeVaryingDisp<VFImg>(24);
      f->SetInput(in);
      f->SetSize(in->GetLargestPossibleRegion().GetSize());
      f->SetOutputSpacing(in->GetSpacing());
      f->SetOutputOrigin(in->GetOrigin());
      f->SetSubsamplingFactor(8);
      f->Update();
    },
    [&]() {
      auto f = itk::InverseDisplacementFieldImageFilter<VDImg, VDImg>::New();
      auto in = MakeVaryingDisp<VDImg>(24);
      f->SetInput(in);
      f->SetSize(in->GetLargestPossibleRegion().GetSize());
      f->SetOutputSpacing(in->GetSpacing());
      f->SetOutputOrigin(in->GetOrigin());
      f->SetSubsamplingFactor(8);
      f->Update();
    },
    runs);
  BenchTime(
    "DisplacementField",
    "TimeVaryingVelocityFieldIntegrationImageFilter",
    [&]() {
      auto f = itk::TimeVaryingVelocityFieldIntegrationImageFilter<VF3, VFImg>::New();
      f->SetInput(tvfF);
      f->SetLowerTimeBound(0.0);
      f->SetUpperTimeBound(1.0);
      f->SetNumberOfIntegrationSteps(2);
      f->Update();
    },
    [&]() {
      auto f = itk::TimeVaryingVelocityFieldIntegrationImageFilter<VD3, VDImg>::New();
      f->SetInput(tvfD);
      f->SetLowerTimeBound(0.0);
      f->SetUpperTimeBound(1.0);
      f->SetNumberOfIntegrationSteps(2);
      f->Update();
    },
    runs);

  BenchTime(
    "ImageFusion",
    "LabelMapContourOverlayImageFilter",
    [&]() {
      auto f = itk::LabelMapContourOverlayImageFilter<ShapeLM, FImg, RGBImg>::New();
      f->SetInput(shapeMap->Clone());
      f->SetFeatureImage(smallF);
      f->SetOpacity(0.4);
      RGBImg::SizeType th;
      th.Fill(1);
      f->SetContourThickness(th);
      f->Update();
    },
    [&]() {
      auto f = itk::LabelMapContourOverlayImageFilter<ShapeLM, DImg, RGBImg>::New();
      f->SetInput(shapeMap->Clone());
      f->SetFeatureImage(smallD);
      f->SetOpacity(0.4);
      RGBImg::SizeType th;
      th.Fill(1);
      f->SetContourThickness(th);
      f->Update();
    },
    runs);

  BenchTime(
    "MathematicalMorphology",
    "MaskedMovingHistogramImageFilter",
    [&]() {
      using Hist = itk::Function::RankHistogram<float>;
      using F = itk::MaskedMovingHistogramImageFilter<FImg, U8Img, FImg, Kernel, Hist>;
      auto f = F::New();
      Kernel::RadiusType rad;
      rad.Fill(1);
      f->SetInput(smallF);
      f->SetMaskImage(mask8);
      f->SetKernel(Kernel::Box(rad));
      f->Update();
    },
    [&]() {
      using Hist = itk::Function::RankHistogram<double>;
      using F = itk::MaskedMovingHistogramImageFilter<DImg, U8Img, DImg, Kernel, Hist>;
      auto f = F::New();
      Kernel::RadiusType rad;
      rad.Fill(1);
      f->SetInput(smallD);
      f->SetMaskImage(mask8);
      f->SetKernel(Kernel::Box(rad));
      f->Update();
    },
    runs);

  BenchTime(
    "ImageFilterBase",
    "CastImageFilter",
    [&]() {
      auto f = itk::CastImageFilter<FImg, FImg>::New();
      f->SetInput(smallF);
      f->Update();
    },
    [&]() {
      auto f = itk::CastImageFilter<DImg, DImg>::New();
      f->SetInput(smallD);
      f->Update();
    },
    runs);

  BenchTime(
    "ConnectedComponents",
    "VectorConnectedComponentImageFilter",
    [&]() {
      auto f = itk::VectorConnectedComponentImageFilter<VFImg, U16Img>::New();
      f->SetInput(dispF);
      f->SetDistanceThreshold(0.2);
      f->Update();
    },
    [&]() {
      auto f = itk::VectorConnectedComponentImageFilter<VDImg, U16Img>::New();
      f->SetInput(dispD);
      f->SetDistanceThreshold(0.2);
      f->Update();
    },
    runs);

  BenchTime(
    "Classifiers",
    "BayesianClassifierImageFilter",
    [&]() {
      auto init = itk::BayesianClassifierInitializationImageFilter<FImg>::New();
      init->SetInput(smallF);
      init->SetNumberOfClasses(3);
      init->Update();
      using VecImg = itk::VectorImage<float, 2>;
      auto f = itk::BayesianClassifierImageFilter<VecImg, unsigned char>::New();
      f->SetInput(init->GetOutput());
      f->Update();
    },
    [&]() {
      auto init = itk::BayesianClassifierInitializationImageFilter<DImg>::New();
      init->SetInput(smallD);
      init->SetNumberOfClasses(3);
      init->Update();
      using VecImg = itk::VectorImage<float, 2>;
      auto f = itk::BayesianClassifierImageFilter<VecImg, unsigned char>::New();
      f->SetInput(init->GetOutput());
      f->Update();
    },
    runs);

  BenchTime(
    "Voronoi",
    "VoronoiSegmentationRGBImageFilter",
    [&]() {
      using F = itk::VoronoiSegmentationRGBImageFilter<V3ImgF, U8Img>;
      auto f = F::New();
      f->SetInput(rgbVec);
      auto prior = itk::BinaryThresholdImageFilter<FImg, U8Img>::New();
      prior->SetInput(smallF);
      prior->SetLowerThreshold(80);
      prior->SetInsideValue(1);
      prior->SetOutsideValue(0);
      prior->Update();
      f->TakeAPrior(prior->GetOutput());
      f->Update();
    },
    [&]() {
      using F = itk::VoronoiSegmentationRGBImageFilter<V3ImgF, U8Img>;
      auto f = F::New();
      f->SetInput(rgbVec);
      auto prior = itk::BinaryThresholdImageFilter<FImg, U8Img>::New();
      prior->SetInput(smallF);
      prior->SetLowerThreshold(80);
      prior->SetInsideValue(1);
      prior->SetOutsideValue(0);
      prior->Update();
      f->TakeAPrior(prior->GetOutput());
      f->Update();
    },
    runs);

  BenchTime(
    "KLMRegionGrowing",
    "KLMRegionGrowImageFilter",
    [&]() {
      auto f = itk::KLMRegionGrowImageFilter<V1ImgF, V1ImgF>::New();
      f->SetInput(v1F);
      typename itk::KLMRegionGrowImageFilter<V1ImgF, V1ImgF>::GridSizeType g;
      g.Fill(8);
      f->SetGridSize(g);
      f->SetMaximumNumberOfRegions(8);
      f->SetMaximumLambda(1000);
      f->SetNumberOfRegions(8);
      f->Update();
    },
    [&]() {
      auto f = itk::KLMRegionGrowImageFilter<V1ImgD, V1ImgD>::New();
      f->SetInput(v1D);
      typename itk::KLMRegionGrowImageFilter<V1ImgD, V1ImgD>::GridSizeType g;
      g.Fill(8);
      f->SetGridSize(g);
      f->SetMaximumNumberOfRegions(8);
      f->SetMaximumLambda(1000);
      f->SetNumberOfRegions(8);
      f->Update();
    },
    runs);

  BenchTime(
    "LevelSets",
    "VectorThresholdSegmentationLevelSetImageFilter",
    [&]() {
      using Feat = itk::Image<itk::Vector<float, 2>, 2>;
      auto feat = Feat::New();
      feat->SetRegions(smallF->GetLargestPossibleRegion());
      feat->CopyInformation(smallF);
      feat->Allocate();
      itk::ImageRegionConstIterator<FImg> a(smallF, smallF->GetLargestPossibleRegion());
      itk::ImageRegionIterator<Feat>      b(feat, feat->GetLargestPossibleRegion());
      for (a.GoToBegin(), b.GoToBegin(); !a.IsAtEnd(); ++a, ++b)
      {
        itk::Vector<float, 2> v;
        v[0] = a.Get();
        v[1] = a.Get();
        b.Set(v);
      }
      using VT = itk::VectorThresholdSegmentationLevelSetImageFilter<FImg, Feat, float>;
      auto f = VT::New();
      f->SetInput(initLSF);
      f->SetFeatureImage(feat);
      typename VT::MeanVectorType mean(2);
      mean.Fill(80.f);
      f->SetMean(mean);
      typename VT::CovarianceMatrixType cov(2, 2);
      cov.Fill(0);
      cov(0, 0) = 400.f;
      cov(1, 1) = 400.f;
      f->SetCovariance(cov);
      f->SetThreshold(2.0);
      f->SetNumberOfIterations(4);
      f->SetMaximumRMSError(1.0);
      f->Update();
    },
    [&]() {
      using Feat = itk::Image<itk::Vector<double, 2>, 2>;
      auto feat = Feat::New();
      feat->SetRegions(smallD->GetLargestPossibleRegion());
      feat->CopyInformation(smallD);
      feat->Allocate();
      itk::ImageRegionConstIterator<DImg> a(smallD, smallD->GetLargestPossibleRegion());
      itk::ImageRegionIterator<Feat>      b(feat, feat->GetLargestPossibleRegion());
      for (a.GoToBegin(), b.GoToBegin(); !a.IsAtEnd(); ++a, ++b)
      {
        itk::Vector<double, 2> v;
        v[0] = a.Get();
        v[1] = a.Get();
        b.Set(v);
      }
      using VT = itk::VectorThresholdSegmentationLevelSetImageFilter<DImg, Feat, double>;
      auto f = VT::New();
      f->SetInput(initLSD);
      f->SetFeatureImage(feat);
      typename VT::MeanVectorType mean(2);
      mean.Fill(80.0);
      f->SetMean(mean);
      typename VT::CovarianceMatrixType cov(2, 2);
      cov.Fill(0);
      cov(0, 0) = 400.0;
      cov(1, 1) = 400.0;
      f->SetCovariance(cov);
      f->SetThreshold(2.0);
      f->SetNumberOfIterations(4);
      f->SetMaximumRMSError(1.0);
      f->Update();
    },
    runs);
  BenchTime(
    "LevelSets",
    "ImplicitManifoldNormalVectorFilter",
    [&]() {
      using Node = itk::NormalBandNode<FImg>;
      using Sparse = itk::SparseImage<Node, 2>;
      using F = itk::ImplicitManifoldNormalVectorFilter<FImg, Sparse>;
      using Fn = itk::NormalVectorDiffusionFunction<Sparse>;
      auto fn = Fn::New();
      auto f = F::New();
      f->SetInput(initLSF);
      f->SetNormalFunction(fn);
      f->SetIsoLevelLow(-4.0f);
      f->SetIsoLevelHigh(4.0f);
      f->SetMaxIteration(4);
      f->Update();
    },
    [&]() {
      using Node = itk::NormalBandNode<DImg>;
      using Sparse = itk::SparseImage<Node, 2>;
      using F = itk::ImplicitManifoldNormalVectorFilter<DImg, Sparse>;
      using Fn = itk::NormalVectorDiffusionFunction<Sparse>;
      auto fn = Fn::New();
      auto f = F::New();
      f->SetInput(initLSD);
      f->SetNormalFunction(fn);
      f->SetIsoLevelLow(-4.0);
      f->SetIsoLevelHigh(4.0);
      f->SetMaxIteration(4);
      f->Update();
    },
    runs);

  return 0;
}
