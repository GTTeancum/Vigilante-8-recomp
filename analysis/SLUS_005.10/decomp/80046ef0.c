// addr: 0x80046ef0  name: S_SVA_OBJ_16C

void S_SVA_OBJ_16C(undefined4 param_1,ushort param_2)

{
  word wVar1;
  uint uVar2;
  ushort uVar3;
  ushort uVar4;
  uint *unaff_s0;
  uint unaff_s1;
  int unaff_s2;
  uint unaff_s3;
  uint unaff_s4;
  ushort *unaff_s5;
  
code_r0x80046ef0:
  uVar4 = 0xe000;
  goto S_SVA_OBJ_170;
  do {
    switch((int)(((ushort)unaff_s0[3] - 1) * 0x10000) >> 0x10) {
    case 0:
      uVar4 = 0x8000;
    default:
S_SVA_OBJ_170:
      do {
        if (uVar4 != 0) {
          if (0x7f < (short)unaff_s0[2]) {
            S_SVA_OBJ_1A0(uVar4,0x7f);
            return;
          }
          if ((short)unaff_s0[2] < 0) {
            param_2 = 0;
          }
        }
        *(ushort *)((int)&VOICE_00_LEFT_RIGHT + unaff_s3 * 2) = param_2 | uVar4;
        do {
          if ((unaff_s2 != 0) || ((unaff_s1 & 2) != 0)) {
            uVar4 = 0;
            uVar3 = *(ushort *)((int)unaff_s0 + 10) & 0x7fff;
            if ((unaff_s2 != 0) || ((unaff_s1 & 8) != 0)) {
              switch((int)((*(ushort *)((int)unaff_s0 + 0xe) - 1) * 0x10000) >> 0x10) {
              case 0:
                uVar4 = 0x8000;
                break;
              case 1:
                uVar4 = 0x9000;
                break;
              case 2:
                uVar4 = 0xa000;
                break;
              case 3:
                uVar4 = 0xb000;
                break;
              case 4:
                uVar4 = 0xc000;
                break;
              case 5:
                uVar4 = 0xd000;
                break;
              case 6:
                uVar4 = 0xe000;
              }
            }
            if (uVar4 != 0) {
              if (0x7f < *(short *)((int)unaff_s0 + 10)) {
                S_SVA_OBJ_280(uVar4,0x7f);
                return;
              }
              if (*(short *)((int)unaff_s0 + 10) < 0) {
                uVar3 = 0;
              }
            }
            *(ushort *)((int)&VOICE_00_LEFT_RIGHT + unaff_s3 * 2 + 2) = uVar3 | uVar4;
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x80) != 0)) {
            _spu_FsetRXXa(unaff_s3 | 3,unaff_s0[7]);
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x10000) != 0)) {
            _spu_FsetRXXa(unaff_s3 | 7,unaff_s0[8]);
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x20000) != 0)) {
            (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] = *(word *)((int)unaff_s0 + 0x3a);
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x40000) != 0)) {
            (&DAT_1f801c0a)[unaff_s3] = (short)unaff_s0[0xf];
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x800) != 0)) {
            uVar4 = (ushort)unaff_s0[0xc];
            if (0x7f < uVar4) {
              uVar4 = 0x7f;
            }
            uVar3 = 0;
            if (((unaff_s2 != 0) || ((unaff_s1 & 0x100) != 0)) && (unaff_s0[9] == 5)) {
              uVar3 = 0x80;
            }
            (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] =
                 (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] & 0xff | (uVar4 | uVar3) << 8;
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x1000) != 0)) {
            uVar4 = *(ushort *)((int)unaff_s0 + 0x32);
            if (0xf < uVar4) {
              uVar4 = 0xf;
            }
            (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] =
                 (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] & 0xff0f | uVar4 << 4;
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x2000) != 0)) {
            uVar4 = (ushort)unaff_s0[0xd];
            if (0x7f < uVar4) {
              uVar4 = 0x7f;
            }
            uVar3 = 0x100;
            if ((unaff_s2 != 0) || ((unaff_s1 & 0x200) != 0)) {
              uVar2 = unaff_s0[10];
              if (uVar2 == 5) {
                S_SVA_OBJ_480();
                return;
              }
              if ((int)uVar2 < 6) {
                if (uVar2 == 1) {
                  S_SVA_OBJ_480();
                  return;
                }
                S_SVA_OBJ_480();
                return;
              }
              if (uVar2 != 7) {
                S_SVA_OBJ_480();
                return;
              }
              uVar3 = 0x300;
            }
            (&DAT_1f801c0a)[unaff_s3] = (&DAT_1f801c0a)[unaff_s3] & 0x3f | (uVar4 | uVar3) << 6;
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x4000) != 0)) {
            uVar4 = *(ushort *)((int)unaff_s0 + 0x36);
            if (0x1f < uVar4) {
              uVar4 = 0x1f;
            }
            uVar3 = 0;
            if ((((unaff_s2 != 0) || ((unaff_s1 & 0x400) != 0)) && (unaff_s0[0xb] != 3)) &&
               (unaff_s0[0xb] == 7)) {
              uVar3 = 0x20;
            }
            (&DAT_1f801c0a)[unaff_s3] = (&DAT_1f801c0a)[unaff_s3] & 0xffc0 | uVar4 | uVar3;
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x8000) != 0)) {
            uVar4 = (ushort)unaff_s0[0xe];
            if (0xf < uVar4) {
              uVar4 = 0xf;
            }
            (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] =
                 (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[unaff_s3] & 0xfff0 | uVar4;
          }
          do {
            unaff_s4 = unaff_s4 + 1;
            unaff_s5 = unaff_s5 + 1;
            if (0x17 < (int)unaff_s4) {
              S_SVA_OBJ_5C0();
              return;
            }
          } while ((*unaff_s0 & 1 << (unaff_s4 & 0x1f)) == 0);
          unaff_s3 = unaff_s4 * 8;
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x10) != 0)) {
            (&VOICE_00_ADPCM_SAMPLE_RATE)[unaff_s4 * 8] = (word)unaff_s0[5];
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x40) != 0)) {
            *unaff_s5 = (ushort)unaff_s0[6];
          }
          if ((unaff_s2 != 0) || ((unaff_s1 & 0x20) != 0)) {
            wVar1 = _spu_note2pitch(*unaff_s5 >> 8,*unaff_s5 & 0xff,
                                    *(ushort *)((int)unaff_s0 + 0x16) >> 8,
                                    *(ushort *)((int)unaff_s0 + 0x16) & 0xff);
            (&VOICE_00_ADPCM_SAMPLE_RATE)[unaff_s4 * 8] = wVar1;
          }
        } while ((unaff_s2 == 0) && ((unaff_s1 & 1) == 0));
        uVar4 = 0;
        param_2 = (ushort)unaff_s0[2] & 0x7fff;
      } while ((unaff_s2 == 0) && ((unaff_s1 & 4) == 0));
      break;
    case 1:
      uVar4 = 0x9000;
      break;
    case 2:
      uVar4 = 0xa000;
      break;
    case 3:
      uVar4 = 0xb000;
      break;
    case 4:
      uVar4 = 0xc000;
      break;
    case 5:
      uVar4 = 0xd000;
      break;
    case 6:
      goto code_r0x80046ef0;
    }
  } while( true );
}

