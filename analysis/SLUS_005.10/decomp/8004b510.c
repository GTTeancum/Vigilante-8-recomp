// addr: 0x8004b510  name: CDREAD2_OBJ_4C

void CDREAD2_OBJ_4C(void)

{
  CdDataCallback(data_ready_callback);
  CdReadyCallback(CDREAD2_OBJ_84);
  CdControl('\x1b',(u_char *)0x0,(u_char *)0x0);
  return;
}

