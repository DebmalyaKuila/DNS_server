#!/bin/bash

PORT=8080

if ss -lun | grep -q ":$PORT"
then
    echo "[OK] DNS port $PORT listening"
else
    echo "[CRITICAL] DNS port $PORT not open"
    exit 1
fi