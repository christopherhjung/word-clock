- edit wifi connection at filesystem/config.json
- edit upload.sh insert serial connection descriptor SERIAL=...

```
bash setup.sh
bash upload.sh
```

http://<ip>/foreground?value=rgb(255,0,0)
http://<ip>/background?value=rgb(255,0,0)
http://<ip>/it_is?value=on/off
http://<ip>/power?value=on/off
http://<ip>/brightness?value=[0.0, 1.0]