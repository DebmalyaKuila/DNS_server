#!/bin/bash

PID_FILE="../dns_server.pid"

if [ ! -f "$PID_FILE" ]; then
    echo "[CRITICAL] PID file not found. DNS server likely not running"
    exit 1
fi

PID=$(cat "$PID_FILE")

if ps -p "$PID" > /dev/null 2>&1; then
    echo "[OK] DNS server running (PID $PID)"
else
    echo "[CRITICAL] DNS server not running"
    exit 1
fi