  ld IX, uart_config
  di
  djnz uart_config
;
uart_config:
    dl 115200
    db 8

