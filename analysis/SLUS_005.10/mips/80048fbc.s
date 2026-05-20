# addr: 0x80048fbc  name: CdReadyCallback
80048fbc:  lui v0,0x8006
80048fc0:  lw v0,0x80(v0)
80048fc4:  lui at,0x8006
80048fc8:  jr ra
80048fcc:  _sw a0,0x80(at)
