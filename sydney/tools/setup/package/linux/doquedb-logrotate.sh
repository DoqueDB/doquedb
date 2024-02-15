%INSTALL_PATH%/log/*.log {
    weekly
    rotate 12
    missingok
    sharedscripts
    postrotate
        /sbin/service doquedb reload > /dev/null 2> /dev/null || true
    endscript
}
