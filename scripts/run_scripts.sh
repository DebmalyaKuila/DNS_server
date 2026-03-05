#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "===== DNS SERVER HEALTH CHECK ====="
echo

bash "$SCRIPT_DIR/check_process.sh"
bash "$SCRIPT_DIR/check_port.sh"
bash "$SCRIPT_DIR/check_dns_query.sh"

echo
echo "===== CHECKS COMPLETED ====="