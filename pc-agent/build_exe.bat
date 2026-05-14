@echo off
echo [*] Dang cai dat thu vien...
pip install -r requirements.txt

echo [*] Dang dong goi file EXE...
pyinstaller --onefile --noconsole --name "pc_agent" src/agent.py

echo [OK] Hoan thanh! File EXE nam trong thu muc 'dist'.
pause
