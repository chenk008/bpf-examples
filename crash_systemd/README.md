##异常重现##
journalctl -fu kubelet

systemctl restart crond





```

Broadcast message from systemd-journald@iZbp10p5dkh35w1si2nykiZ (Tue 2022-07-05 14:33:20 CST):

systemd[1]: Failed to run main loop: Input/output error


Broadcast message from systemd-journald@iZbp10p5dkh35w1si2nykiZ (Tue 2022-07-05 14:33:20 CST):

systemd[1]: Freezing execution.


Message from syslogd@iZbp10p5dkh35w1si2nykiZ at Jul  5 14:33:20 ...
 systemd[1]:Failed to run main loop: Input/output error

Message from syslogd@iZbp10p5dkh35w1si2nykiZ at Jul  5 14:33:20 ...
 systemd[1]:Freezing execution.
```