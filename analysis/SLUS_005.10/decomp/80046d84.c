// addr: 0x80046d84  name: SpuSetVoiceAttr

void SpuSetVoiceAttr(SpuVoiceAttr *arg)

{
  short sVar1;
  bool bVar2;
  word wVar3;
  int iVar4;
  ushort uVar5;
  ushort uVar6;
  uint uVar7;
  uint uVar8;
  ushort *puVar9;
  
  uVar8 = 0;
  uVar7 = arg->mask;
  puVar9 = &DAT_8005ed90;
  bVar2 = uVar7 == 0;
  do {
    if ((arg->voice & 1 << (uVar8 & 0x1f)) != 0) {
      if ((bVar2) || ((uVar7 & 0x10) != 0)) {
        (&VOICE_00_ADPCM_SAMPLE_RATE)[uVar8 * 8] = arg->pitch;
      }
      if ((bVar2) || ((uVar7 & 0x40) != 0)) {
        *puVar9 = arg->sample_note;
      }
      if ((bVar2) || ((uVar7 & 0x20) != 0)) {
        wVar3 = _spu_note2pitch(*puVar9 >> 8,*puVar9 & 0xff,arg->note >> 8,arg->note & 0xff);
        (&VOICE_00_ADPCM_SAMPLE_RATE)[uVar8 * 8] = wVar3;
      }
      if ((bVar2) || ((uVar7 & 1) != 0)) {
        uVar6 = 0;
        uVar5 = (arg->volume).left & 0x7fff;
        if ((bVar2) || ((uVar7 & 4) != 0)) {
          switch((int)(((ushort)(arg->volmode).left - 1) * 0x10000) >> 0x10) {
          case 0:
            uVar6 = 0x8000;
            break;
          case 1:
            uVar6 = 0x9000;
            break;
          case 2:
            uVar6 = 0xa000;
            break;
          case 3:
            uVar6 = 0xb000;
            break;
          case 4:
            uVar6 = 0xc000;
            break;
          case 5:
            uVar6 = 0xd000;
            break;
          case 6:
            uVar6 = 0xe000;
          }
        }
        if (uVar6 != 0) {
          sVar1 = (arg->volume).left;
          if (0x7f < sVar1) {
            S_SVA_OBJ_1A0(uVar6,0x7f);
            return;
          }
          if (sVar1 < 0) {
            uVar5 = 0;
          }
        }
        *(ushort *)(&VOICE_00_LEFT_RIGHT + uVar8 * 4) = uVar5 | uVar6;
      }
      if ((bVar2) || ((uVar7 & 2) != 0)) {
        uVar6 = 0;
        uVar5 = (arg->volume).right & 0x7fff;
        if ((bVar2) || ((uVar7 & 8) != 0)) {
          switch((int)(((ushort)(arg->volmode).right - 1) * 0x10000) >> 0x10) {
          case 0:
            uVar6 = 0x8000;
            break;
          case 1:
            uVar6 = 0x9000;
            break;
          case 2:
            uVar6 = 0xa000;
            break;
          case 3:
            uVar6 = 0xb000;
            break;
          case 4:
            uVar6 = 0xc000;
            break;
          case 5:
            uVar6 = 0xd000;
            break;
          case 6:
            uVar6 = 0xe000;
          }
        }
        if (uVar6 != 0) {
          sVar1 = (arg->volume).right;
          if (0x7f < sVar1) {
            S_SVA_OBJ_280(uVar6,0x7f);
            return;
          }
          if (sVar1 < 0) {
            uVar5 = 0;
          }
        }
        *(ushort *)((int)&VOICE_00_LEFT_RIGHT + uVar8 * 0x10 + 2) = uVar5 | uVar6;
      }
      if ((bVar2) || ((uVar7 & 0x80) != 0)) {
        _spu_FsetRXXa(uVar8 << 3 | 3,arg->addr);
      }
      if ((bVar2) || ((uVar7 & 0x10000) != 0)) {
        _spu_FsetRXXa(uVar8 << 3 | 7,arg->loop_addr);
      }
      if ((bVar2) || ((uVar7 & 0x20000) != 0)) {
        (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] = arg->adsr1;
      }
      if ((bVar2) || ((uVar7 & 0x40000) != 0)) {
        (&DAT_1f801c0a)[uVar8 * 8] = arg->adsr2;
      }
      if ((bVar2) || ((uVar7 & 0x800) != 0)) {
        uVar6 = arg->ar;
        if (0x7f < uVar6) {
          uVar6 = 0x7f;
        }
        uVar5 = 0;
        if (((bVar2) || ((uVar7 & 0x100) != 0)) && (arg->a_mode == 5)) {
          uVar5 = 0x80;
        }
        (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] =
             (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] & 0xff | (uVar6 | uVar5) << 8;
      }
      if ((bVar2) || ((uVar7 & 0x1000) != 0)) {
        uVar6 = arg->dr;
        if (0xf < uVar6) {
          uVar6 = 0xf;
        }
        (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] =
             (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] & 0xff0f | uVar6 << 4;
      }
      if ((bVar2) || ((uVar7 & 0x2000) != 0)) {
        uVar6 = arg->sr;
        if (0x7f < uVar6) {
          uVar6 = 0x7f;
        }
        uVar5 = 0x100;
        if ((bVar2) || ((uVar7 & 0x200) != 0)) {
          iVar4 = arg->s_mode;
          if (iVar4 == 5) {
            S_SVA_OBJ_480();
            return;
          }
          if (iVar4 < 6) {
            if (iVar4 == 1) {
              S_SVA_OBJ_480();
              return;
            }
            S_SVA_OBJ_480();
            return;
          }
          if (iVar4 != 7) {
            S_SVA_OBJ_480();
            return;
          }
          uVar5 = 0x300;
        }
        (&DAT_1f801c0a)[uVar8 * 8] = (&DAT_1f801c0a)[uVar8 * 8] & 0x3f | (uVar6 | uVar5) << 6;
      }
      if ((bVar2) || ((uVar7 & 0x4000) != 0)) {
        uVar6 = arg->rr;
        if (0x1f < uVar6) {
          uVar6 = 0x1f;
        }
        uVar5 = 0;
        if ((((bVar2) || ((uVar7 & 0x400) != 0)) && (arg->r_mode != 3)) && (arg->r_mode == 7)) {
          uVar5 = 0x20;
        }
        (&DAT_1f801c0a)[uVar8 * 8] = (&DAT_1f801c0a)[uVar8 * 8] & 0xffc0 | uVar6 | uVar5;
      }
      if ((bVar2) || ((uVar7 & 0x8000) != 0)) {
        uVar6 = arg->sl;
        if (0xf < uVar6) {
          uVar6 = 0xf;
        }
        (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] =
             (&VOICE_00_ADSR_ATT_DEC_SUS_REL)[uVar8 * 8] & 0xfff0 | uVar6;
      }
    }
    uVar8 = uVar8 + 1;
    puVar9 = puVar9 + 1;
    if (0x17 < (int)uVar8) {
      S_SVA_OBJ_5C0();
      return;
    }
  } while( true );
}

