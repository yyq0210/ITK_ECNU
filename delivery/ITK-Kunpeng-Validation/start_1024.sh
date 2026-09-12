#!/bin/bash
sed -i 's/\r$//' /tmp/run_1024_remeasure.sh
chmod +x /tmp/run_1024_remeasure.sh
nohup bash /tmp/run_1024_remeasure.sh >/tmp/run_1024.log 2>&1 &
echo STARTED:$!
sleep 1
head -5 /tmp/run_1024.log || true
