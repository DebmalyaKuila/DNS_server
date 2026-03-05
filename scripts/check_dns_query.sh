#!/bin/bash

SERVER="127.0.0.1"
PORT="8080"
DOMAIN="example.com"

RESPONSE=$(dig @$SERVER -p $PORT $DOMAIN +short 2>/dev/null)

if [ $? -ne 0 ] || [ -z "$RESPONSE" ]; then
    echo "[CRITICAL] DNS query failed"
    exit 1
else
    echo "[OK] DNS query successful: $RESPONSE"
fi