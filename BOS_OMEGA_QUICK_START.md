# BOS-OMEGA Quick Start Guide

## 1. Build the C Runtime

```bash
cd /home/ubuntu
gcc bos_omega_production.c -o bos_omega_prod -lcurl
chmod +x bos_omega_prod
```

## 2. Test the Runtime

```bash
# Test PowerShell
./bos_omega_prod run "Get-Date"

# Test Android (if device connected)
./bos_omega_prod run "pm list packages"

# Test Language Advisor
./bos_omega_prod lang "build web app" "beginner" "web" "linux" "1 week" "simple" "react"
```

## 3. Run the Web Application

```bash
cd bos-omega-ui
pnpm install
pnpm dev
```

Open browser: `http://localhost:3000`

## 4. API Endpoints

### Create Task
```bash
curl -X POST http://localhost:3000/api/tasks \
  -H "Content-Type: application/json" \
  -d '{"objective": "List installed packages"}'
```

### Execute Task
```bash
curl -X POST http://localhost:3000/api/tasks/1/execute \
  -H "Content-Type: application/json" \
  -d '{"command": "pm list packages"}'
```

### Get Audit Logs
```bash
curl http://localhost:3000/api/audit-logs
```

## 5. WebSocket Connection

```javascript
const ws = new WebSocket('ws://localhost:3000');

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log('Update:', data);
};
```

## 6. Key Features

- **Real-time Task Monitoring**: WebSocket-driven updates
- **Risk Assessment**: Automatic classification of destructive commands
- **Audit Trail**: Complete logging of all operations
- **Multi-Tool Support**: PowerShell, ADB, Remote APIs
- **ChatGPT-Style UI**: Intuitive, modern interface

## 7. Example Commands

### PowerShell
```
Get-ChildItem /home/ubuntu
Get-Date
Get-Process
```

### Android (ADB)
```
pm list packages
input tap 500 500
am start -n com.example.app/.MainActivity
```

### Language Advisor
```
lang "build AI system" "expert" "web" "linux" "4 weeks" "fast" "no deps"
```

## 8. Troubleshooting

### PowerShell not found
```bash
sudo apt-get install -y powershell
```

### ADB not found
```bash
sudo apt-get install -y adb
```

### Port 3000 already in use
```bash
lsof -i :3000
kill -9 <PID>
```

---

For full documentation, see `BOS_OMEGA_COMPLETE_SOURCE.md`
