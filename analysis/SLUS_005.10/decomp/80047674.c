// addr: 0x80047674  name: FUN_80047674

/* Possible S_SCCV.OBJ/SpuSetCommonCDVolume */

void FUN_80047674(word param_1,word param_2)

{
  CD_VOL_L = param_1;
  CD_VOL_R = param_2;
  return;
}

