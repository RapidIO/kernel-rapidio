/***************************************************************************
** (C) Copyright 2011; Integrated Device Technology
** July 5, 2011 All Rights Reserved.
**
** This file contains the IDT TSI721 register definitions and bitfield masks. 
**  
** Disclaimer
** Integrated Device Technology, Inc. ("IDT") reserves the right to make changes
** to its products or specifications at any time, without notice, in order to
** improve design or performance. IDT does not assume responsibility for use of
** any circuitry described herein other than the circuitry embodied in an IDT
** product. Disclosure of the information herein does not convey a license or
** any other right, by implication or otherwise, in any patent, trademark, or
** other intellectual property right of IDT. IDT products may contain errata
** which can affect product performance to a minor or immaterial degree. Current
** characterized errata will be made available upon request. Items identified
** herein as "reserved" or "undefined" are reserved for future definition. IDT
** does not assume responsibility for conflicts or incompatibilities arising
** from the future definition of such items. IDT products have not been
** designed, tested, or manufactured for use in, and thus are not warranted for,
** applications where the failure, malfunction, or any inaccuracy in the
** application carries a risk of death, serious bodily injury, or damage to
** tangible property. Code examples provided herein by IDT are for illustrative
** purposes only and should not be relied upon for developing applications. Any
** use of such code examples shall be at the user's sole risk.
***************************************************************************/

#include "RapidIO_Utilities_API.h"

#ifndef __TSI721_H__
#define __TSI721_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************/
/* TSI721 : PCIe Register address offset definitions */
/*****************************************************/

/* NOTE: this offset must be applied to PCIe register addresses when accessed via
 * S-RIO maintenance writes/reads */
#define TSI721_PCI_SRIO_MAINT_OFFSET                     ((uint32_t)0x00070000)

#define TSI721_PCI_ID                                    ((uint32_t)0x00000000)
#define TSI721_PCI_CSR                                   ((uint32_t)0x00000004)
#define TSI721_PCI_CLASS                                 ((uint32_t)0x00000008)
#define TSI721_PCI_MISC0                                 ((uint32_t)0x0000000c)
#define TSI721_PCI_BAR0                                  ((uint32_t)0x00000010)
#define TSI721_PCI_BAR1                                  ((uint32_t)0x00000014)
#define TSI721_PCI_BAR2                                  ((uint32_t)0x00000018)
#define TSI721_PCI_BAR3                                  ((uint32_t)0x0000001c)
#define TSI721_PCI_BAR4                                  ((uint32_t)0x00000020)
#define TSI721_PCI_BAR5                                  ((uint32_t)0x00000024)
#define TSI721_PCI_CCISPTR                               ((uint32_t)0x00000028)
#define TSI721_PCI_SID                                   ((uint32_t)0x0000002c)
#define TSI721_PCI_EROMBASE                              ((uint32_t)0x00000030)
#define TSI721_PCI_CAPPTR                                ((uint32_t)0x00000034)
#define TSI721_PCI_MISC1                                 ((uint32_t)0x0000003c)
#define TSI721_PCIECAP                                   ((uint32_t)0x00000040)
#define TSI721_PCIEDCAP                                  ((uint32_t)0x00000044)
#define TSI721_PCIEDCTL                                  ((uint32_t)0x00000048)
#define TSI721_PCIELCAP                                  ((uint32_t)0x0000004c)
#define TSI721_PCIELCTL                                  ((uint32_t)0x00000050)
#define TSI721_PCIEDCAP2                                 ((uint32_t)0x00000064)
#define TSI721_PCIEDCTL2                                 ((uint32_t)0x00000068)
#define TSI721_PCIELCAP2                                 ((uint32_t)0x0000006c)
#define TSI721_PCIELCTL2                                 ((uint32_t)0x00000070)
#define TSI721_MSIXCAP                                   ((uint32_t)0x000000a0)
#define TSI721_MSIXTBL                                   ((uint32_t)0x000000a4)
#define TSI721_MSIXPBA                                   ((uint32_t)0x000000a8)
#define TSI721_PMCAP                                     ((uint32_t)0x000000c0)
#define TSI721_PMCSR                                     ((uint32_t)0x000000c4)
#define TSI721_MSICAP                                    ((uint32_t)0x000000d0)
#define TSI721_MSIADDR                                   ((uint32_t)0x000000d4)
#define TSI721_MSIUADDR                                  ((uint32_t)0x000000d8)
#define TSI721_MSIMDATA                                  ((uint32_t)0x000000dc)
#define TSI721_MSIMASK                                   ((uint32_t)0x000000e0)
#define TSI721_MSIPENDING                                ((uint32_t)0x000000e4)
#define TSI721_SSIDSSVIDCAP                              ((uint32_t)0x000000f0)
#define TSI721_SSIDSSVID                                 ((uint32_t)0x000000f4)
#define TSI721_ECFGADDR                                  ((uint32_t)0x000000f8)
#define TSI721_ECFGDATA                                  ((uint32_t)0x000000fc)
#define TSI721_AERCAP                                    ((uint32_t)0x00000100)
#define TSI721_AERUES                                    ((uint32_t)0x00000104)
#define TSI721_AERUEM                                    ((uint32_t)0x00000108)
#define TSI721_AERUESV                                   ((uint32_t)0x0000010c)
#define TSI721_AERCES                                    ((uint32_t)0x00000110)
#define TSI721_AERCEM                                    ((uint32_t)0x00000114)
#define TSI721_AERCTL                                    ((uint32_t)0x00000118)
#define TSI721_AERHL1DW                                  ((uint32_t)0x0000011c)
#define TSI721_AERHL2DW                                  ((uint32_t)0x00000120)
#define TSI721_AERHL3DW                                  ((uint32_t)0x00000124)
#define TSI721_AERHL4DW                                  ((uint32_t)0x00000128)
#define TSI721_SNUMCAP                                   ((uint32_t)0x00000180)
#define TSI721_SNUMLDW                                   ((uint32_t)0x00000184)
#define TSI721_SNUMUDW                                   ((uint32_t)0x00000188)
#define TSI721_EPCTL                                     ((uint32_t)0x00000400)
#define TSI721_EPSTS                                     ((uint32_t)0x00000404)
#define TSI721_SEDELAY                                   ((uint32_t)0x0000040c)
#define TSI721_BARSETUP0                                 ((uint32_t)0x00000440)
#define TSI721_BARSETUP1                                 ((uint32_t)0x00000444)
#define TSI721_BARSETUP2                                 ((uint32_t)0x00000448)
#define TSI721_BARSETUP3                                 ((uint32_t)0x0000044c)
#define TSI721_BARSETUP4                                 ((uint32_t)0x00000450)
#define TSI721_BARSETUP5                                 ((uint32_t)0x00000454)
#define TSI721_IERRORCTL                                 ((uint32_t)0x00000480)
#define TSI721_IERRORSTS0                                ((uint32_t)0x00000484)
#define TSI721_IERRORMSK0                                ((uint32_t)0x00000488)
#define TSI721_IERRORSEV0                                ((uint32_t)0x0000048c)
#define TSI721_IERRORTST0                                ((uint32_t)0x00000494)
#define TSI721_TOCTL                                     ((uint32_t)0x000004b0)
#define TSI721_IFBTOCNT                                  ((uint32_t)0x000004b4)
#define TSI721_EFBTOCNT                                  ((uint32_t)0x000004b8)
#define TSI721_TOTSCTL                                   ((uint32_t)0x000004bc)
#define TSI721_MECTL                                     ((uint32_t)0x000004c0)
#define TSI721_SERDESCFG                                 ((uint32_t)0x00000510)
#define TSI721_SERDESSTS0                                ((uint32_t)0x00000514)
#define TSI721_LANESTS0                                  ((uint32_t)0x0000051c)
#define TSI721_LANESTS1                                  ((uint32_t)0x00000520)
#define TSI721_LANESTS2                                  ((uint32_t)0x00000524)
#define TSI721_PHYFSMT0                                  ((uint32_t)0x00000528)
#define TSI721_PHYFSMT1                                  ((uint32_t)0x0000052c)
#define TSI721_PHYLCFG0                                  ((uint32_t)0x00000530)
#define TSI721_PHYLCFG1                                  ((uint32_t)0x00000534)
#define TSI721_PHYLSTS0                                  ((uint32_t)0x00000538)
#define TSI721_PHYLSTS1                                  ((uint32_t)0x0000053c)
#define TSI721_PHYLSTATE0                                ((uint32_t)0x00000540)
#define TSI721_PHYLTSSMSTS0                              ((uint32_t)0x00000544)
#define TSI721_PHYLTSSMSTS1                              ((uint32_t)0x00000548)
#define TSI721_PHYCNT0                                   ((uint32_t)0x0000054c)
#define TSI721_PHYCNT1                                   ((uint32_t)0x00000550)
#define TSI721_PHYCNTCFG                                 ((uint32_t)0x00000554)
#define TSI721_PHYRECEL                                  ((uint32_t)0x00000558)
#define TSI721_PHYPRBS                                   ((uint32_t)0x0000055c)
#define TSI721_DLCTL1                                    ((uint32_t)0x00000600)
#define TSI721_DLCTL2                                    ((uint32_t)0x00000604)
#define TSI721_DLCTL3                                    ((uint32_t)0x00000608)
#define TSI721_DLSTS                                     ((uint32_t)0x0000060c)
#define TSI721_DLRXSTS                                   ((uint32_t)0x00000610)
#define TSI721_DLTXSTS                                   ((uint32_t)0x00000614)
#define TSI721_DLCNT0                                    ((uint32_t)0x00000618)
#define TSI721_DLCNT1                                    ((uint32_t)0x0000061c)
#define TSI721_DLCNTCFG                                  ((uint32_t)0x00000620)
#define TSI721_TLSTSE                                    ((uint32_t)0x00000680)
#define TSI721_TLCTL                                     ((uint32_t)0x00000684)
#define TSI721_TLCNT0                                    ((uint32_t)0x00000688)
#define TSI721_TLCNT1                                    ((uint32_t)0x0000068c)
#define TSI721_TLCNTCFG                                  ((uint32_t)0x00000690)
#define TSI721_INTSTS                                    ((uint32_t)0x000006a0)
#define TSI721_PMPC0                                     ((uint32_t)0x00000700)
#define TSI721_PMPC1                                     ((uint32_t)0x00000704)
#define TSI721_FCVC0PTCC                                 ((uint32_t)0x00000800)
#define TSI721_FCVC0NPCC                                 ((uint32_t)0x00000804)
#define TSI721_FCVC0CPCC                                 ((uint32_t)0x00000808)
#define TSI721_FCVC0PTCL                                 ((uint32_t)0x0000080c)
#define TSI721_FCVC0NPCL                                 ((uint32_t)0x00000810)
#define TSI721_FCVC0CPCL                                 ((uint32_t)0x00000814)
#define TSI721_FCVC0PTCA                                 ((uint32_t)0x00000818)
#define TSI721_FCVC0NPCA                                 ((uint32_t)0x0000081c)
#define TSI721_FCVC0CPCA                                 ((uint32_t)0x00000820)
#define TSI721_FCVC0PTCR                                 ((uint32_t)0x00000824)
#define TSI721_FCVC0NPCR                                 ((uint32_t)0x00000828)
#define TSI721_FCVC0CPCR                                 ((uint32_t)0x0000082c)
#define TSI721_EFBTC                                     ((uint32_t)0x00000860)
#define TSI721_IFBCNT0                                   ((uint32_t)0x000008b0)
#define TSI721_IFBCNT1                                   ((uint32_t)0x000008b4)
#define TSI721_IFBCNTCFG                                 ((uint32_t)0x000008b8)
#define TSI721_EFBCNT0                                   ((uint32_t)0x000008c0)
#define TSI721_EFBCNT1                                   ((uint32_t)0x000008c4)
#define TSI721_EFBCNTCFG                                 ((uint32_t)0x000008c8)
#define TSI721_UEEM                                      ((uint32_t)0x00000d90)
#define TSI721_CEEM                                      ((uint32_t)0x00000d94)
#define TSI721_STMCTL                                    ((uint32_t)0x00000e54)
#define TSI721_STMSTS                                    ((uint32_t)0x00000e58)
#define TSI721_STMTCTL                                   ((uint32_t)0x00000e5c)
#define TSI721_STMTSTS                                   ((uint32_t)0x00000e60)
#define TSI721_STMECNT0                                  ((uint32_t)0x00000e64)
#define TSI721_STMECNT1                                  ((uint32_t)0x00000e68)
#define TSI721_STMECNT2                                  ((uint32_t)0x00000e6c)
#define TSI721_STMECNT3                                  ((uint32_t)0x00000e70)
#define TSI721_ALLCS                                     ((uint32_t)0x00000e84)
#define TSI721_IFBVC0PTCFG                               ((uint32_t)0x00000e90)
#define TSI721_IFBVC0NPCFG                               ((uint32_t)0x00000e94)
#define TSI721_IFBVC0CPCFG                               ((uint32_t)0x00000e98)
#define TSI721_IFCSTS                                    ((uint32_t)0x00000ea8)
#define TSI721_EFBVC0PTSTS                               ((uint32_t)0x00000ec0)
#define TSI721_EFBVC0NPSTS                               ((uint32_t)0x00000ec4)
#define TSI721_EFBVC0CPSTS                               ((uint32_t)0x00000ec8)
#define TSI721_EFBRBSTS                                  ((uint32_t)0x00000ecc)

/*****************************************************/
/* TSI721 : PCIe Register Bit Masks and Reset Values */
/*           definitions for every register          */
/*****************************************************/

/* TSI721_PCI_ID : Register Bits Masks Definitions */
#define TSI721_PCI_ID_VID                                 ((uint32_t)0x0000ffff)
#define TSI721_PCI_ID_DID                                 ((uint32_t)0xffff0000)

/* TSI721_PCI_CSR : Register Bits Masks Definitions */
#define TSI721_PCI_CSR_IOAE                               ((uint32_t)0x00000001)
#define TSI721_PCI_CSR_MAE                                ((uint32_t)0x00000002)
#define TSI721_PCI_CSR_BME                                ((uint32_t)0x00000004)
#define TSI721_PCI_CSR_SCE                                ((uint32_t)0x00000008)
#define TSI721_PCI_CSR_MWI                                ((uint32_t)0x00000010)
#define TSI721_PCI_CSR_VGAS                               ((uint32_t)0x00000020)
#define TSI721_PCI_CSR_PERRE                              ((uint32_t)0x00000040)
#define TSI721_PCI_CSR_ADSTEP                             ((uint32_t)0x00000080)
#define TSI721_PCI_CSR_SERRE                              ((uint32_t)0x00000100)
#define TSI721_PCI_CSR_FB2BE                              ((uint32_t)0x00000200)
#define TSI721_PCI_CSR_INTXD                              ((uint32_t)0x00000400)
#define TSI721_PCI_CSR_INTS                               ((uint32_t)0x00080000)
#define TSI721_PCI_CSR_CAPL                               ((uint32_t)0x00100000)
#define TSI721_PCI_CSR_C66MHZ                             ((uint32_t)0x00200000)
#define TSI721_PCI_CSR_FB2B                               ((uint32_t)0x00800000)
#define TSI721_PCI_CSR_MDPED                              ((uint32_t)0x01000000)
#define TSI721_PCI_CSR_DEVT                               ((uint32_t)0x06000000)
#define TSI721_PCI_CSR_STAS                               ((uint32_t)0x08000000)
#define TSI721_PCI_CSR_RTAS                               ((uint32_t)0x10000000)
#define TSI721_PCI_CSR_RMAS                               ((uint32_t)0x20000000)
#define TSI721_PCI_CSR_SSE                                ((uint32_t)0x40000000)
#define TSI721_PCI_CSR_DPE                                ((uint32_t)0x80000000)

/* TSI721_PCI_CLASS : Register Bits Masks Definitions */
#define TSI721_PCI_CLASS_RID                              ((uint32_t)0x000000ff)
#define TSI721_PCI_CLASS_INTF                             ((uint32_t)0x0000ff00)
#define TSI721_PCI_CLASS_SUB                              ((uint32_t)0x00ff0000)
#define TSI721_PCI_CLASS_BASE                             ((uint32_t)0xff000000)

/* TSI721_PCI_MISC0 : Register Bits Masks Definitions */
#define TSI721_PCI_MISC0_CLS                              ((uint32_t)0x000000ff)
#define TSI721_PCI_MISC0_LTIMER                           ((uint32_t)0x0000ff00)
#define TSI721_PCI_MISC0_HDR                              ((uint32_t)0x00ff0000)
#define TSI721_PCI_MISC0_CCODE                            ((uint32_t)0x0f000000)
#define TSI721_PCI_MISC0_START                            ((uint32_t)0x40000000)
#define TSI721_PCI_MISC0_CAPABLE                          ((uint32_t)0x80000000)

/* TSI721_PCI_BAR0 : Register Bits Masks Definitions */
#define TSI721_PCI_BAR0_MEMSI                             ((uint32_t)0x00000001)
#define TSI721_PCI_BAR0_TYPE                              ((uint32_t)0x00000006)
#define TSI721_PCI_BAR0_PREF                              ((uint32_t)0x00000008)
#define TSI721_PCI_BAR0_BADDR                             ((uint32_t)0xfffffff0)

/* TSI721_PCI_BAR1 : Register Bits Masks Definitions */
#define TSI721_PCI_BAR1_MEMSI                             ((uint32_t)0x00000001)
#define TSI721_PCI_BAR1_TYPE                              ((uint32_t)0x00000006)
#define TSI721_PCI_BAR1_PREF                              ((uint32_t)0x00000008)
#define TSI721_PCI_BAR1_BADDR                             ((uint32_t)0xfffffff0)

/* TSI721_PCI_BAR2 : Register Bits Masks Definitions */
#define TSI721_PCI_BAR2_MEMSI                             ((uint32_t)0x00000001)
#define TSI721_PCI_BAR2_TYPE                              ((uint32_t)0x00000006)
#define TSI721_PCI_BAR2_PREF                              ((uint32_t)0x00000008)
#define TSI721_PCI_BAR2_BADDR                             ((uint32_t)0xfffffff0)

/* TSI721_PCI_BAR3 : Register Bits Masks Definitions */
#define TSI721_PCI_BAR3_MEMSI                             ((uint32_t)0x00000001)
#define TSI721_PCI_BAR3_TYPE                              ((uint32_t)0x00000006)
#define TSI721_PCI_BAR3_PREF                              ((uint32_t)0x00000008)
#define TSI721_PCI_BAR3_BADDR                             ((uint32_t)0xfffffff0)

/* TSI721_PCI_BAR4 : Register Bits Masks Definitions */
#define TSI721_PCI_BAR4_MEMSI                             ((uint32_t)0x00000001)
#define TSI721_PCI_BAR4_TYPE                              ((uint32_t)0x00000006)
#define TSI721_PCI_BAR4_PREF                              ((uint32_t)0x00000008)
#define TSI721_PCI_BAR4_BADDR                             ((uint32_t)0xfffffff0)

/* TSI721_PCI_BAR5 : Register Bits Masks Definitions */
#define TSI721_PCI_BAR5_MEMSI                             ((uint32_t)0x00000001)
#define TSI721_PCI_BAR5_TYPE                              ((uint32_t)0x00000006)
#define TSI721_PCI_BAR5_PREF                              ((uint32_t)0x00000008)
#define TSI721_PCI_BAR5_BADDR                             ((uint32_t)0xfffffff0)

/* TSI721_PCI_CCISPTR : Register Bits Masks Definitions */
#define TSI721_PCI_CCISPTR_CCISPTR                        ((uint32_t)0xffffffff)

/* TSI721_PCI_SID : Register Bits Masks Definitions */
#define TSI721_PCI_SID_SUBVID                             ((uint32_t)0x0000ffff)
#define TSI721_PCI_SID_SUBID                              ((uint32_t)0xffff0000)

/* TSI721_PCI_EROMBASE : Register Bits Masks Definitions */
#define TSI721_PCI_EROMBASE_EN                            ((uint32_t)0x00000001)
#define TSI721_PCI_EROMBASE_BADDR                         ((uint32_t)0xfffff800)

/* TSI721_PCI_CAPPTR : Register Bits Masks Definitions */
#define TSI721_PCI_CAPPTR_CAPPTR                          ((uint32_t)0x000000ff)

/* TSI721_PCI_MISC1 : Register Bits Masks Definitions */
#define TSI721_PCI_MISC1_INTRLINE                         ((uint32_t)0x000000ff)
#define TSI721_PCI_MISC1_INTRPIN                          ((uint32_t)0x0000ff00)
#define TSI721_PCI_MISC1_MINGNT                           ((uint32_t)0x00ff0000)
#define TSI721_PCI_MISC1_MAXLAT                           ((uint32_t)0xff000000)

/* TSI721_PCIECAP : Register Bits Masks Definitions */
#define TSI721_PCIECAP_CAPID                              ((uint32_t)0x000000ff)
#define TSI721_PCIECAP_NXTPTR                             ((uint32_t)0x0000ff00)
#define TSI721_PCIECAP_VER                                ((uint32_t)0x000f0000)
#define TSI721_PCIECAP_TYPE                               ((uint32_t)0x00f00000)
#define TSI721_PCIECAP_SLOT                               ((uint32_t)0x01000000)
#define TSI721_PCIECAP_IMN                                ((uint32_t)0x3e000000)

/* TSI721_PCIEDCAP : Register Bits Masks Definitions */
#define TSI721_PCIEDCAP_MPAYLOAD                          ((uint32_t)0x00000007)
#define TSI721_PCIEDCAP_PFS                               ((uint32_t)0x00000018)
#define TSI721_PCIEDCAP_ETAG                              ((uint32_t)0x00000020)
#define TSI721_PCIEDCAP_E0AL                              ((uint32_t)0x000001c0)
#define TSI721_PCIEDCAP_E1AL                              ((uint32_t)0x00000e00)
#define TSI721_PCIEDCAP_ABP                               ((uint32_t)0x00001000)
#define TSI721_PCIEDCAP_AIP                               ((uint32_t)0x00002000)
#define TSI721_PCIEDCAP_PIP                               ((uint32_t)0x00004000)
#define TSI721_PCIEDCAP_RBERR                             ((uint32_t)0x00008000)
#define TSI721_PCIEDCAP_CSPLV                             ((uint32_t)0x03fc0000)
#define TSI721_PCIEDCAP_CSPLS                             ((uint32_t)0x0c000000)
#define TSI721_PCIEDCAP_FLR                               ((uint32_t)0x10000000)

/* TSI721_PCIEDCTL : Register Bits Masks Definitions */
#define TSI721_PCIEDCTL_CEREN                             ((uint32_t)0x00000001)
#define TSI721_PCIEDCTL_NFEREN                            ((uint32_t)0x00000002)
#define TSI721_PCIEDCTL_FEREN                             ((uint32_t)0x00000004)
#define TSI721_PCIEDCTL_URREN                             ((uint32_t)0x00000008)
#define TSI721_PCIEDCTL_ERO                               ((uint32_t)0x00000010)
#define TSI721_PCIEDCTL_MPS                               ((uint32_t)0x000000e0)
#define TSI721_PCIEDCTL_ETFEN                             ((uint32_t)0x00000100)
#define TSI721_PCIEDCTL_PFEN                              ((uint32_t)0x00000200)
#define TSI721_PCIEDCTL_AUXPMEN                           ((uint32_t)0x00000400)
#define TSI721_PCIEDCTL_NOSNOOP                           ((uint32_t)0x00000800)
#define TSI721_PCIEDCTL_MRRS                              ((uint32_t)0x00007000)
#define TSI721_PCIEDCTL_IFLR                              ((uint32_t)0x00008000)
#define TSI721_PCIEDCTL_CED                               ((uint32_t)0x00010000)
#define TSI721_PCIEDCTL_NFED                              ((uint32_t)0x00020000)
#define TSI721_PCIEDCTL_FED                               ((uint32_t)0x00040000)
#define TSI721_PCIEDCTL_URD                               ((uint32_t)0x00080000)
#define TSI721_PCIEDCTL_AUXPD                             ((uint32_t)0x00100000)
#define TSI721_PCIEDCTL_TP                                ((uint32_t)0x00200000)

/* TSI721_PCIELCAP : Register Bits Masks Definitions */
#define TSI721_PCIELCAP_MAXLNKSPD                         ((uint32_t)0x0000000f)
#define TSI721_PCIELCAP_MAXLNKWDTH                        ((uint32_t)0x000003f0)
#define TSI721_PCIELCAP_ASPMS                             ((uint32_t)0x00000c00)
#define TSI721_PCIELCAP_L0SEL                             ((uint32_t)0x00007000)
#define TSI721_PCIELCAP_L1EL                              ((uint32_t)0x00038000)
#define TSI721_PCIELCAP_CPM                               ((uint32_t)0x00040000)
#define TSI721_PCIELCAP_SDERR                             ((uint32_t)0x00080000)
#define TSI721_PCIELCAP_DLLLA                             ((uint32_t)0x00100000)
#define TSI721_PCIELCAP_LBN                               ((uint32_t)0x00200000)
#define TSI721_PCIELCAP_PORTNUM                           ((uint32_t)0xff000000)

/* TSI721_PCIELCTL : Register Bits Masks Definitions */
#define TSI721_PCIELCTL_ASPM                              ((uint32_t)0x00000003)
#define TSI721_PCIELCTL_RCB                               ((uint32_t)0x00000008)
#define TSI721_PCIELCTL_LDIS                              ((uint32_t)0x00000010)
#define TSI721_PCIELCTL_LRET                              ((uint32_t)0x00000020)
#define TSI721_PCIELCTL_CCLK                              ((uint32_t)0x00000040)
#define TSI721_PCIELCTL_ESYNC                             ((uint32_t)0x00000080)
#define TSI721_PCIELCTL_CLKPWRMGT                         ((uint32_t)0x00000100)
#define TSI721_PCIELCTL_HAWD                              ((uint32_t)0x00000200)
#define TSI721_PCIELCTL_LBWINTEN                          ((uint32_t)0x00000400)
#define TSI721_PCIELCTL_LABWINTEN                         ((uint32_t)0x00000800)
#define TSI721_PCIELCTL_CLS                               ((uint32_t)0x000f0000)
#define TSI721_PCIELCTL_NLW                               ((uint32_t)0x03f00000)
#define TSI721_PCIELCTL_LTRAIN                            ((uint32_t)0x08000000)
#define TSI721_PCIELCTL_SCLK                              ((uint32_t)0x10000000)
#define TSI721_PCIELCTL_DLLLA                             ((uint32_t)0x20000000)
#define TSI721_PCIELCTL_LBWSTS                            ((uint32_t)0x40000000)
#define TSI721_PCIELCTL_LABWSTS                           ((uint32_t)0x80000000)

/* TSI721_PCIEDCAP2 : Register Bits Masks Definitions */
#define TSI721_PCIEDCAP2_CTRS                             ((uint32_t)0x0000000f)
#define TSI721_PCIEDCAP2_CTDS                             ((uint32_t)0x00000010)
#define TSI721_PCIEDCAP2_ARIFS                            ((uint32_t)0x00000020)
#define TSI721_PCIEDCAP2_ATOPRS                           ((uint32_t)0x00000040)
#define TSI721_PCIEDCAP2_ATOPC32S                         ((uint32_t)0x00000080)
#define TSI721_PCIEDCAP2_ATOPC64S                         ((uint32_t)0x00000100)
#define TSI721_PCIEDCAP2_CASC128S                         ((uint32_t)0x00000200)
#define TSI721_PCIEDCAP2_NROEP                            ((uint32_t)0x00000400)
#define TSI721_PCIEDCAP2_LTRMS                            ((uint32_t)0x00000800)
#define TSI721_PCIEDCAP2_TPHCS                            ((uint32_t)0x00003000)
#define TSI721_PCIEDCAP2_EFMTFS                           ((uint32_t)0x00100000)
#define TSI721_PCIEDCAP2_E2ETPS                           ((uint32_t)0x00200000)

/* TSI721_PCIEDCTL2 : Register Bits Masks Definitions */
#define TSI721_PCIEDCTL2_CTV                              ((uint32_t)0x0000000f)
#define TSI721_PCIEDCTL2_CTD                              ((uint32_t)0x00000010)
#define TSI721_PCIEDCTL2_ARIFEN                           ((uint32_t)0x00000020)
#define TSI721_PCIEDCTL2_ATOPRE                           ((uint32_t)0x00000040)
#define TSI721_PCIEDCTL2_ATOPEB                           ((uint32_t)0x00000080)
#define TSI721_PCIEDCTL2_IDORE                            ((uint32_t)0x00000100)
#define TSI721_PCIEDCTL2_IDOCE                            ((uint32_t)0x00000200)
#define TSI721_PCIEDCTL2_LTRME                            ((uint32_t)0x00000400)
#define TSI721_PCIEDCTL2_E2ETLPPB                         ((uint32_t)0x00008000)

/* TSI721_PCIELCAP2 : Register Bits Masks Definitions */

/* TSI721_PCIELCTL2 : Register Bits Masks Definitions */
#define TSI721_PCIELCTL2_TLS                              ((uint32_t)0x0000000f)
#define TSI721_PCIELCTL2_ECOMP                            ((uint32_t)0x00000010)
#define TSI721_PCIELCTL2_HASD                             ((uint32_t)0x00000020)
#define TSI721_PCIELCTL2_SDE                              ((uint32_t)0x00000040)
#define TSI721_PCIELCTL2_TM                               ((uint32_t)0x00000380)
#define TSI721_PCIELCTL2_EMC                              ((uint32_t)0x00000400)
#define TSI721_PCIELCTL2_CSOS                             ((uint32_t)0x00000800)
#define TSI721_PCIELCTL2_COMP_DE                          ((uint32_t)0x00001000)
#define TSI721_PCIELCTL2_CDE                              ((uint32_t)0x00010000)

/* TSI721_MSIXCAP : Register Bits Masks Definitions */
#define TSI721_MSIXCAP_CAPID                              ((uint32_t)0x000000ff)
#define TSI721_MSIXCAP_NXTPTR                             ((uint32_t)0x0000ff00)
#define TSI721_MSIXCAP_TBLSIZE                            ((uint32_t)0x07ff0000)
#define TSI721_MSIXCAP_MASK                               ((uint32_t)0x40000000)
#define TSI721_MSIXCAP_EN                                 ((uint32_t)0x80000000)

/* TSI721_MSIXTBL : Register Bits Masks Definitions */
#define TSI721_MSIXTBL_BIR                                ((uint32_t)0x00000007)
#define TSI721_MSIXTBL_OFFSET                             ((uint32_t)0xfffffff8)

/* TSI721_MSIXPBA : Register Bits Masks Definitions */
#define TSI721_MSIXPBA_BIR                                ((uint32_t)0x00000007)
#define TSI721_MSIXPBA_OFFSET                             ((uint32_t)0xfffffff8)

/* TSI721_PMCAP : Register Bits Masks Definitions */
#define TSI721_PMCAP_CAPID                                ((uint32_t)0x000000ff)
#define TSI721_PMCAP_NXTPTR                               ((uint32_t)0x0000ff00)
#define TSI721_PMCAP_VER                                  ((uint32_t)0x00070000)
#define TSI721_PMCAP_PMECLK                               ((uint32_t)0x00080000)
#define TSI721_PMCAP_DEVSP                                ((uint32_t)0x00200000)
#define TSI721_PMCAP_AUXI                                 ((uint32_t)0x01c00000)
#define TSI721_PMCAP_D1                                   ((uint32_t)0x02000000)
#define TSI721_PMCAP_D2                                   ((uint32_t)0x04000000)
#define TSI721_PMCAP_PME                                  ((uint32_t)0xf8000000)

/* TSI721_PMCSR : Register Bits Masks Definitions */
#define TSI721_PMCSR_PSTATE                               ((uint32_t)0x00000003)
#define TSI721_PMCSR_NOSOFTRST                            ((uint32_t)0x00000008)
#define TSI721_PMCSR_PMEE                                 ((uint32_t)0x00000100)
#define TSI721_PMCSR_DSEL                                 ((uint32_t)0x00001e00)
#define TSI721_PMCSR_DSCALE                               ((uint32_t)0x00006000)
#define TSI721_PMCSR_PMES                                 ((uint32_t)0x00008000)
#define TSI721_PMCSR_B2B3                                 ((uint32_t)0x00400000)
#define TSI721_PMCSR_BPCCE                                ((uint32_t)0x00800000)
#define TSI721_PMCSR_DATA                                 ((uint32_t)0xff000000)

/* TSI721_MSICAP : Register Bits Masks Definitions */
#define TSI721_MSICAP_CAPID                               ((uint32_t)0x000000ff)
#define TSI721_MSICAP_NXTPTR                              ((uint32_t)0x0000ff00)
#define TSI721_MSICAP_EN                                  ((uint32_t)0x00010000)
#define TSI721_MSICAP_MMC                                 ((uint32_t)0x000e0000)
#define TSI721_MSICAP_MME                                 ((uint32_t)0x00700000)
#define TSI721_MSICAP_A64                                 ((uint32_t)0x00800000)
#define TSI721_MSICAP_MASKCAP                             ((uint32_t)0x01000000)

/* TSI721_MSIADDR : Register Bits Masks Definitions */
#define TSI721_MSIADDR_ADDR                               ((uint32_t)0xfffffffc)

/* TSI721_MSIUADDR : Register Bits Masks Definitions */
#define TSI721_MSIUADDR_UADDR                             ((uint32_t)0xffffffff)

/* TSI721_MSIMDATA : Register Bits Masks Definitions */
#define TSI721_MSIMDATA_MDATA                             ((uint32_t)0x0000ffff)

/* TSI721_MSIMASK : Register Bits Masks Definitions */
#define TSI721_MSIMASK_MASK                               ((uint32_t)0xffffffff)

/* TSI721_MSIPENDING : Register Bits Masks Definitions */
#define TSI721_MSIPENDING_PENDING                         ((uint32_t)0xffffffff)

/* TSI721_SSIDSSVIDCAP : Register Bits Masks Definitions */
#define TSI721_SSIDSSVIDCAP_CAPID                         ((uint32_t)0x000000ff)
#define TSI721_SSIDSSVIDCAP_NXTPTR                        ((uint32_t)0x0000ff00)

/* TSI721_SSIDSSVID : Register Bits Masks Definitions */
#define TSI721_SSIDSSVID_SSVID                            ((uint32_t)0x0000ffff)
#define TSI721_SSIDSSVID_SSID                             ((uint32_t)0xffff0000)

/* TSI721_ECFGADDR : Register Bits Masks Definitions */
#define TSI721_ECFGADDR_REG                               ((uint32_t)0x000000fc)
#define TSI721_ECFGADDR_EREG                              ((uint32_t)0x00000f00)

/* TSI721_ECFGDATA : Register Bits Masks Definitions */
#define TSI721_ECFGDATA_DATA                              ((uint32_t)0xffffffff)

/* TSI721_AERCAP : Register Bits Masks Definitions */
#define TSI721_AERCAP_CAPID                               ((uint32_t)0x0000ffff)
#define TSI721_AERCAP_CAPVER                              ((uint32_t)0x000f0000)
#define TSI721_AERCAP_NXTPTR                              ((uint32_t)0xfff00000)

/* TSI721_AERUES : Register Bits Masks Definitions */
#define TSI721_AERUES_UDEF                                ((uint32_t)0x00000001)
#define TSI721_AERUES_DLPERR                              ((uint32_t)0x00000010)
#define TSI721_AERUES_SDOENERR                            ((uint32_t)0x00000020)
#define TSI721_AERUES_POISONED                            ((uint32_t)0x00001000)
#define TSI721_AERUES_FCPERR                              ((uint32_t)0x00002000)
#define TSI721_AERUES_COMPTO                              ((uint32_t)0x00004000)
#define TSI721_AERUES_CABORT                              ((uint32_t)0x00008000)
#define TSI721_AERUES_UECOMP                              ((uint32_t)0x00010000)
#define TSI721_AERUES_RCVOVR                              ((uint32_t)0x00020000)
#define TSI721_AERUES_MALFORMED                           ((uint32_t)0x00040000)
#define TSI721_AERUES_ECRC                                ((uint32_t)0x00080000)
#define TSI721_AERUES_UR                                  ((uint32_t)0x00100000)
#define TSI721_AERUES_ACSV                                ((uint32_t)0x00200000)
#define TSI721_AERUES_UIE                                 ((uint32_t)0x00400000)
#define TSI721_AERUES_MCBLKTLP                            ((uint32_t)0x00800000)
#define TSI721_AERUES_ATOPEB                              ((uint32_t)0x01000000)
#define TSI721_AERUES_TLPPBE                              ((uint32_t)0x02000000)

/* TSI721_AERUEM : Register Bits Masks Definitions */
#define TSI721_AERUEM_UDEF                                ((uint32_t)0x00000001)
#define TSI721_AERUEM_DLPERR                              ((uint32_t)0x00000010)
#define TSI721_AERUEM_SDOENERR                            ((uint32_t)0x00000020)
#define TSI721_AERUEM_POISONED                            ((uint32_t)0x00001000)
#define TSI721_AERUEM_FCPERR                              ((uint32_t)0x00002000)
#define TSI721_AERUEM_COMPTO                              ((uint32_t)0x00004000)
#define TSI721_AERUEM_CABORT                              ((uint32_t)0x00008000)
#define TSI721_AERUEM_UECOMP                              ((uint32_t)0x00010000)
#define TSI721_AERUEM_RCVOVR                              ((uint32_t)0x00020000)
#define TSI721_AERUEM_MALFORMED                           ((uint32_t)0x00040000)
#define TSI721_AERUEM_ECRC                                ((uint32_t)0x00080000)
#define TSI721_AERUEM_UR                                  ((uint32_t)0x00100000)
#define TSI721_AERUEM_ACSV                                ((uint32_t)0x00200000)
#define TSI721_AERUEM_UIE                                 ((uint32_t)0x00400000)
#define TSI721_AERUEM_MCBLKTLP                            ((uint32_t)0x00800000)
#define TSI721_AERUEM_ATOPEB                              ((uint32_t)0x01000000)
#define TSI721_AERUEM_TLPPBE                              ((uint32_t)0x02000000)

/* TSI721_AERUESV : Register Bits Masks Definitions */
#define TSI721_AERUESV_UDEF                               ((uint32_t)0x00000001)
#define TSI721_AERUESV_DLPERR                             ((uint32_t)0x00000010)
#define TSI721_AERUESV_SDOENERR                           ((uint32_t)0x00000020)
#define TSI721_AERUESV_POISONED                           ((uint32_t)0x00001000)
#define TSI721_AERUESV_FCPERR                             ((uint32_t)0x00002000)
#define TSI721_AERUESV_COMPTO                             ((uint32_t)0x00004000)
#define TSI721_AERUESV_CABORT                             ((uint32_t)0x00008000)
#define TSI721_AERUESV_UECOMP                             ((uint32_t)0x00010000)
#define TSI721_AERUESV_RCVOVR                             ((uint32_t)0x00020000)
#define TSI721_AERUESV_MALFORMED                          ((uint32_t)0x00040000)
#define TSI721_AERUESV_ECRC                               ((uint32_t)0x00080000)
#define TSI721_AERUESV_UR                                 ((uint32_t)0x00100000)
#define TSI721_AERUESV_ACSV                               ((uint32_t)0x00200000)
#define TSI721_AERUESV_UIE                                ((uint32_t)0x00400000)
#define TSI721_AERUESV_MCBLKTLP                           ((uint32_t)0x00800000)
#define TSI721_AERUESV_ATOPEB                             ((uint32_t)0x01000000)
#define TSI721_AERUESV_TLPPBE                             ((uint32_t)0x02000000)

/* TSI721_AERCES : Register Bits Masks Definitions */
#define TSI721_AERCES_RCVERR                              ((uint32_t)0x00000001)
#define TSI721_AERCES_BADTLP                              ((uint32_t)0x00000040)
#define TSI721_AERCES_BADDLLP                             ((uint32_t)0x00000080)
#define TSI721_AERCES_RPLYROVR                            ((uint32_t)0x00000100)
#define TSI721_AERCES_RPLYTO                              ((uint32_t)0x00001000)
#define TSI721_AERCES_ADVISORYNF                          ((uint32_t)0x00002000)
#define TSI721_AERCES_CIE                                 ((uint32_t)0x00004000)
#define TSI721_AERCES_HLO                                 ((uint32_t)0x00008000)

/* TSI721_AERCEM : Register Bits Masks Definitions */
#define TSI721_AERCEM_RCVERR                              ((uint32_t)0x00000001)
#define TSI721_AERCEM_BADTLP                              ((uint32_t)0x00000040)
#define TSI721_AERCEM_BADDLLP                             ((uint32_t)0x00000080)
#define TSI721_AERCEM_RPLYROVR                            ((uint32_t)0x00000100)
#define TSI721_AERCEM_RPLYTO                              ((uint32_t)0x00001000)
#define TSI721_AERCEM_ADVISORYNF                          ((uint32_t)0x00002000)
#define TSI721_AERCEM_CIE                                 ((uint32_t)0x00004000)
#define TSI721_AERCEM_HLO                                 ((uint32_t)0x00008000)

/* TSI721_AERCTL : Register Bits Masks Definitions */
#define TSI721_AERCTL_FEPTR                               ((uint32_t)0x0000001f)
#define TSI721_AERCTL_ECRCGC                              ((uint32_t)0x00000020)
#define TSI721_AERCTL_ECRCGE                              ((uint32_t)0x00000040)
#define TSI721_AERCTL_ECRCCC                              ((uint32_t)0x00000080)
#define TSI721_AERCTL_ECRCCE                              ((uint32_t)0x00000100)
#define TSI721_AERCTL_MHRC                                ((uint32_t)0x00000200)
#define TSI721_AERCTL_MHRE                                ((uint32_t)0x00000400)

/* TSI721_AERHL1DW : Register Bits Masks Definitions */
#define TSI721_AERHL1DW_HL                                ((uint32_t)0xffffffff)

/* TSI721_AERHL2DW : Register Bits Masks Definitions */
#define TSI721_AERHL2DW_HL                                ((uint32_t)0xffffffff)

/* TSI721_AERHL3DW : Register Bits Masks Definitions */
#define TSI721_AERHL3DW_HL                                ((uint32_t)0xffffffff)

/* TSI721_AERHL4DW : Register Bits Masks Definitions */
#define TSI721_AERHL4DW_HL                                ((uint32_t)0xffffffff)

/* TSI721_SNUMCAP : Register Bits Masks Definitions */
#define TSI721_SNUMCAP_CAPID                              ((uint32_t)0x0000ffff)
#define TSI721_SNUMCAP_CAPVER                             ((uint32_t)0x000f0000)
#define TSI721_SNUMCAP_NXTPTR                             ((uint32_t)0xfff00000)

/* TSI721_SNUMLDW : Register Bits Masks Definitions */
#define TSI721_SNUMLDW_SNUM                               ((uint32_t)0xffffffff)

/* TSI721_SNUMUDW : Register Bits Masks Definitions */
#define TSI721_SNUMUDW_SNUM                               ((uint32_t)0xffffffff)

/* TSI721_EPCTL : Register Bits Masks Definitions */
#define TSI721_EPCTL_REGUNLOCK                            ((uint32_t)0x00000001)
#define TSI721_EPCTL_IFBCTDIS                             ((uint32_t)0x00000002)
#define TSI721_EPCTL_EFBCTDIS                             ((uint32_t)0x00000004)

/* TSI721_EPSTS : Register Bits Masks Definitions */
#define TSI721_EPSTS_QUASIRSTSTS                          ((uint32_t)0x00000001)

/* TSI721_SEDELAY : Register Bits Masks Definitions */
#define TSI721_SEDELAY_SEDELAY                            ((uint32_t)0x0000ffff)

/* TSI721_BARSETUP0 : Register Bits Masks Definitions */
#define TSI721_BARSETUP0_MEMSI                            ((uint32_t)0x00000001)
#define TSI721_BARSETUP0_TYPE                             ((uint32_t)0x00000006)
#define TSI721_BARSETUP0_PREF                             ((uint32_t)0x00000008)
#define TSI721_BARSETUP0_SIZE                             ((uint32_t)0x000003f0)
#define TSI721_BARSETUP0_EN                               ((uint32_t)0x80000000)

/* TSI721_BARSETUP1 : Register Bits Masks Definitions */
#define TSI721_BARSETUP1_MEMSI                            ((uint32_t)0x00000001)
#define TSI721_BARSETUP1_TYPE                             ((uint32_t)0x00000006)
#define TSI721_BARSETUP1_PREF                             ((uint32_t)0x00000008)
#define TSI721_BARSETUP1_SIZE                             ((uint32_t)0x000003f0)
#define TSI721_BARSETUP1_EN                               ((uint32_t)0x80000000)

/* TSI721_BARSETUP2 : Register Bits Masks Definitions */
#define TSI721_BARSETUP2_MEMSI                            ((uint32_t)0x00000001)
#define TSI721_BARSETUP2_TYPE                             ((uint32_t)0x00000006)
#define TSI721_BARSETUP2_PREF                             ((uint32_t)0x00000008)
#define TSI721_BARSETUP2_SIZE                             ((uint32_t)0x000003f0)
#define TSI721_BARSETUP2_EN                               ((uint32_t)0x80000000)

/* TSI721_BARSETUP3 : Register Bits Masks Definitions */
#define TSI721_BARSETUP3_MEMSI                            ((uint32_t)0x00000001)
#define TSI721_BARSETUP3_TYPE                             ((uint32_t)0x00000006)
#define TSI721_BARSETUP3_PREF                             ((uint32_t)0x00000008)
#define TSI721_BARSETUP3_SIZE                             ((uint32_t)0x000003f0)
#define TSI721_BARSETUP3_EN                               ((uint32_t)0x80000000)

/* TSI721_BARSETUP4 : Register Bits Masks Definitions */
#define TSI721_BARSETUP4_MEMSI                            ((uint32_t)0x00000001)
#define TSI721_BARSETUP4_TYPE                             ((uint32_t)0x00000006)
#define TSI721_BARSETUP4_PREF                             ((uint32_t)0x00000008)
#define TSI721_BARSETUP4_SIZE                             ((uint32_t)0x000003f0)
#define TSI721_BARSETUP4_EN                               ((uint32_t)0x80000000)

/* TSI721_BARSETUP5 : Register Bits Masks Definitions */
#define TSI721_BARSETUP5_MEMSI                            ((uint32_t)0x00000001)
#define TSI721_BARSETUP5_TYPE                             ((uint32_t)0x00000006)
#define TSI721_BARSETUP5_PREF                             ((uint32_t)0x00000008)
#define TSI721_BARSETUP5_SIZE                             ((uint32_t)0x000003f0)
#define TSI721_BARSETUP5_EN                               ((uint32_t)0x80000000)

/* TSI721_IERRORCTL : Register Bits Masks Definitions */
#define TSI721_IERRORCTL_IERROREN                         ((uint32_t)0x00000001)

/* TSI721_IERRORSTS0 : Register Bits Masks Definitions */
#define TSI721_IERRORSTS0_IFBPTLPTO                       ((uint32_t)0x00000001)
#define TSI721_IERRORSTS0_IFBNPTLPTO                      ((uint32_t)0x00000002)
#define TSI721_IERRORSTS0_IFBCPTLPTO                      ((uint32_t)0x00000004)
#define TSI721_IERRORSTS0_EFBPTLPTO                       ((uint32_t)0x00000010)
#define TSI721_IERRORSTS0_EFBNPTLPTO                      ((uint32_t)0x00000020)
#define TSI721_IERRORSTS0_EFBCPTLPTO                      ((uint32_t)0x00000040)
#define TSI721_IERRORSTS0_IFBDATSBE                       ((uint32_t)0x00000080)
#define TSI721_IERRORSTS0_IFBDATDBE                       ((uint32_t)0x00000100)
#define TSI721_IERRORSTS0_IFBCTLSBE                       ((uint32_t)0x00000200)
#define TSI721_IERRORSTS0_IFBCTLDBE                       ((uint32_t)0x00000400)
#define TSI721_IERRORSTS0_EFBDATSBE                       ((uint32_t)0x00000800)
#define TSI721_IERRORSTS0_EFBDATDBE                       ((uint32_t)0x00001000)
#define TSI721_IERRORSTS0_EFBCTLSBE                       ((uint32_t)0x00002000)
#define TSI721_IERRORSTS0_EFBCTLDBE                       ((uint32_t)0x00004000)
#define TSI721_IERRORSTS0_E2EPE                           ((uint32_t)0x00008000)
#define TSI721_IERRORSTS0_RBCTLSBE                        ((uint32_t)0x00010000)
#define TSI721_IERRORSTS0_RBCTLDBE                        ((uint32_t)0x00020000)

/* TSI721_IERRORMSK0 : Register Bits Masks Definitions */
#define TSI721_IERRORMSK0_IFBPTLPTO                       ((uint32_t)0x00000001)
#define TSI721_IERRORMSK0_IFBNPTLPTO                      ((uint32_t)0x00000002)
#define TSI721_IERRORMSK0_IFBCPTLPTO                      ((uint32_t)0x00000004)
#define TSI721_IERRORMSK0_EFBPTLPTO                       ((uint32_t)0x00000010)
#define TSI721_IERRORMSK0_EFBNPTLPTO                      ((uint32_t)0x00000020)
#define TSI721_IERRORMSK0_EFBCPTLPTO                      ((uint32_t)0x00000040)
#define TSI721_IERRORMSK0_IFBDATSBE                       ((uint32_t)0x00000080)
#define TSI721_IERRORMSK0_IFBDATDBE                       ((uint32_t)0x00000100)
#define TSI721_IERRORMSK0_IFBCTLSBE                       ((uint32_t)0x00000200)
#define TSI721_IERRORMSK0_IFBCTLDBE                       ((uint32_t)0x00000400)
#define TSI721_IERRORMSK0_EFBDATSBE                       ((uint32_t)0x00000800)
#define TSI721_IERRORMSK0_EFBDATDBE                       ((uint32_t)0x00001000)
#define TSI721_IERRORMSK0_EFBCTLSBE                       ((uint32_t)0x00002000)
#define TSI721_IERRORMSK0_EFBCTLDBE                       ((uint32_t)0x00004000)
#define TSI721_IERRORMSK0_E2EPE                           ((uint32_t)0x00008000)
#define TSI721_IERRORMSK0_RBCTLSBE                        ((uint32_t)0x00010000)
#define TSI721_IERRORMSK0_RBCTLDBE                        ((uint32_t)0x00020000)

/* TSI721_IERRORSEV0 : Register Bits Masks Definitions */
#define TSI721_IERRORSEV0_IFBPTLPTO                       ((uint32_t)0x00000001)
#define TSI721_IERRORSEV0_IFBNPTLPTO                      ((uint32_t)0x00000002)
#define TSI721_IERRORSEV0_IFBCPTLPTO                      ((uint32_t)0x00000004)
#define TSI721_IERRORSEV0_EFBPTLPTO                       ((uint32_t)0x00000010)
#define TSI721_IERRORSEV0_EFBNPTLPTO                      ((uint32_t)0x00000020)
#define TSI721_IERRORSEV0_EFBCPTLPTO                      ((uint32_t)0x00000040)
#define TSI721_IERRORSEV0_IFBDATSBE                       ((uint32_t)0x00000080)
#define TSI721_IERRORSEV0_IFBDATDBE                       ((uint32_t)0x00000100)
#define TSI721_IERRORSEV0_IFBCTLSBE                       ((uint32_t)0x00000200)
#define TSI721_IERRORSEV0_IFBCTLDBE                       ((uint32_t)0x00000400)
#define TSI721_IERRORSEV0_EFBDATSBE                       ((uint32_t)0x00000800)
#define TSI721_IERRORSEV0_EFBDATDBE                       ((uint32_t)0x00001000)
#define TSI721_IERRORSEV0_EFBCTLSBE                       ((uint32_t)0x00002000)
#define TSI721_IERRORSEV0_EFBCTLDBE                       ((uint32_t)0x00004000)
#define TSI721_IERRORSEV0_E2EPE                           ((uint32_t)0x00008000)
#define TSI721_IERRORSEV0_RBCTLSBE                        ((uint32_t)0x00010000)
#define TSI721_IERRORSEV0_RBCTLDBE                        ((uint32_t)0x00020000)

/* TSI721_IERRORTST0 : Register Bits Masks Definitions */
#define TSI721_IERRORTST0_IFBPTLPTO                       ((uint32_t)0x00000001)
#define TSI721_IERRORTST0_IFBNPTLPTO                      ((uint32_t)0x00000002)
#define TSI721_IERRORTST0_IFBCPTLPTO                      ((uint32_t)0x00000004)
#define TSI721_IERRORTST0_EFBPTLPTO                       ((uint32_t)0x00000010)
#define TSI721_IERRORTST0_EFBNPTLPTO                      ((uint32_t)0x00000020)
#define TSI721_IERRORTST0_EFBCPTLPTO                      ((uint32_t)0x00000040)
#define TSI721_IERRORTST0_IFBDATSBE                       ((uint32_t)0x00000080)
#define TSI721_IERRORTST0_IFBDATDBE                       ((uint32_t)0x00000100)
#define TSI721_IERRORTST0_IFBCTLSBE                       ((uint32_t)0x00000200)
#define TSI721_IERRORTST0_IFBCTLDBE                       ((uint32_t)0x00000400)
#define TSI721_IERRORTST0_EFBDATSBE                       ((uint32_t)0x00000800)
#define TSI721_IERRORTST0_EFBDATDBE                       ((uint32_t)0x00001000)
#define TSI721_IERRORTST0_EFBCTLSBE                       ((uint32_t)0x00002000)
#define TSI721_IERRORTST0_EFBCTLDBE                       ((uint32_t)0x00004000)
#define TSI721_IERRORTST0_E2EPE                           ((uint32_t)0x00008000)
#define TSI721_IERRORTST0_RBCTLSBE                        ((uint32_t)0x00010000)
#define TSI721_IERRORTST0_RBCTLDBE                        ((uint32_t)0x00020000)

/* TSI721_TOCTL : Register Bits Masks Definitions */
#define TSI721_TOCTL_ETO                                  ((uint32_t)0x00000001)

/* TSI721_IFBTOCNT : Register Bits Masks Definitions */
#define TSI721_IFBTOCNT_IFBPTTOC                          ((uint32_t)0x000000ff)
#define TSI721_IFBTOCNT_IFBNPTOC                          ((uint32_t)0x0000ff00)
#define TSI721_IFBTOCNT_IFBCPTOC                          ((uint32_t)0x00ff0000)

/* TSI721_EFBTOCNT : Register Bits Masks Definitions */
#define TSI721_EFBTOCNT_EFBPTOC                           ((uint32_t)0x000000ff)
#define TSI721_EFBTOCNT_EFBNPTOC                          ((uint32_t)0x0000ff00)
#define TSI721_EFBTOCNT_EFBCPTOC                          ((uint32_t)0x00ff0000)

/* TSI721_TOTSCTL : Register Bits Masks Definitions */
#define TSI721_TOTSCTL_TCOUNT                             ((uint32_t)0x7fc00000)

/* TSI721_MECTL : Register Bits Masks Definitions */
#define TSI721_MECTL_EIEN                                 ((uint32_t)0x00000008)
#define TSI721_MECTL_IFBDATSBE                            ((uint32_t)0x00000100)
#define TSI721_MECTL_IFBCTLSBE                            ((uint32_t)0x00000200)
#define TSI721_MECTL_IFBDATDBE                            ((uint32_t)0x00000400)
#define TSI721_MECTL_IFBCTLDBE                            ((uint32_t)0x00000800)
#define TSI721_MECTL_EFBDATSBE                            ((uint32_t)0x00001000)
#define TSI721_MECTL_EFBCTLSBE                            ((uint32_t)0x00002000)
#define TSI721_MECTL_EFBDATDBE                            ((uint32_t)0x00004000)
#define TSI721_MECTL_EFBCTLDBE                            ((uint32_t)0x00008000)
#define TSI721_MECTL_RBCTLSBE                             ((uint32_t)0x00010000)
#define TSI721_MECTL_RBCTLDBE                             ((uint32_t)0x00020000)

/* TSI721_SERDESCFG : Register Bits Masks Definitions */
#define TSI721_SERDESCFG_RCVD_OVRD                        ((uint32_t)0x0000000f)
#define TSI721_SERDESCFG_FEID                             ((uint32_t)0x00000100)
#define TSI721_SERDESCFG_EIDD                             ((uint32_t)0x00000200)
#define TSI721_SERDESCFG_P1D                              ((uint32_t)0x00000400)
#define TSI721_SERDESCFG_P2D                              ((uint32_t)0x00000800)
#define TSI721_SERDESCFG_ILPBSEL                          ((uint32_t)0x00007000)
#define TSI721_SERDESCFG_LSE                              ((uint32_t)0x00010000)

/* TSI721_SERDESSTS0 : Register Bits Masks Definitions */
#define TSI721_SERDESSTS0_RCVD                            ((uint32_t)0x0000000f)
#define TSI721_SERDESSTS0_CURR_SPEED                      ((uint32_t)0x00000010)
#define TSI721_SERDESSTS0_PLL_LOCK                        ((uint32_t)0x00000020)
#define TSI721_SERDESSTS0_P0_READY                        ((uint32_t)0x00000040)
#define TSI721_SERDESSTS0_CDR_LOCK                        ((uint32_t)0x00000f00)
#define TSI721_SERDESSTS0_EIDLE_DETST                     ((uint32_t)0x0000f000)
#define TSI721_SERDESSTS0_EIDLE_DET                       ((uint32_t)0x000f0000)
#define TSI721_SERDESSTS0_EIOS_DET                        ((uint32_t)0x00f00000)
#define TSI721_SERDESSTS0_EIDLE_INF                       ((uint32_t)0x01000000)
#define TSI721_SERDESSTS0_EIDLE_DETS                      ((uint32_t)0x1e000000)

/* TSI721_LANESTS0 : Register Bits Masks Definitions */
#define TSI721_LANESTS0_PDE                               ((uint32_t)0x0000000f)
#define TSI721_LANESTS0_E8B10B                            ((uint32_t)0x000f0000)

/* TSI721_LANESTS1 : Register Bits Masks Definitions */
#define TSI721_LANESTS1_UND                               ((uint32_t)0x0000000f)
#define TSI721_LANESTS1_OVR                               ((uint32_t)0x000f0000)

/* TSI721_LANESTS2 : Register Bits Masks Definitions */
#define TSI721_LANESTS2_L0DAP                             ((uint32_t)0x0000000f)
#define TSI721_LANESTS2_L1DAP                             ((uint32_t)0x000000f0)
#define TSI721_LANESTS2_L2DAP                             ((uint32_t)0x00000f00)
#define TSI721_LANESTS2_L3DAP                             ((uint32_t)0x0000f000)

/* TSI721_PHYFSMT0 : Register Bits Masks Definitions */
#define TSI721_PHYFSMT0_SSCD                              ((uint32_t)0x00000003)
#define TSI721_PHYFSMT0_USCD                              ((uint32_t)0x0000000c)
#define TSI721_PHYFSMT0_REL                               ((uint32_t)0x00000030)
#define TSI721_PHYFSMT0_EITXPDG1                          ((uint32_t)0x00000f00)
#define TSI721_PHYFSMT0_EIRXPDG1                          ((uint32_t)0x0003f000)
#define TSI721_PHYFSMT0_RXDETDELAY                        ((uint32_t)0x003c0000)
#define TSI721_PHYFSMT0_EITXPDG2                          ((uint32_t)0x03c00000)
#define TSI721_PHYFSMT0_EIRXPDG2                          ((uint32_t)0xfc000000)

/* TSI721_PHYFSMT1 : Register Bits Masks Definitions */
#define TSI721_PHYFSMT1_SOSIP                             ((uint32_t)0x000007ff)
#define TSI721_PHYFSMT1_NFTSNCC                           ((uint32_t)0x0007f800)
#define TSI721_PHYFSMT1_NFTSCC                            ((uint32_t)0x07f80000)
#define TSI721_PHYFSMT1_EIES_FTS                          ((uint32_t)0xf0000000)

/* TSI721_PHYLCFG0 : Register Bits Masks Definitions */
#define TSI721_PHYLCFG0_LNKNUM                            ((uint32_t)0x000000ff)
#define TSI721_PHYLCFG0_G1CME                             ((uint32_t)0x00000100)
#define TSI721_PHYLCFG0_SRMBLDIS                          ((uint32_t)0x00000400)
#define TSI721_PHYLCFG0_CLINKDIS                          ((uint32_t)0x00000800)
#define TSI721_PHYLCFG0_PCEC                              ((uint32_t)0x00001000)
#define TSI721_PHYLCFG0_SCLINKEN                          ((uint32_t)0x00002000)
#define TSI721_PHYLCFG0_ILSCC                             ((uint32_t)0x00004000)
#define TSI721_PHYLCFG0_ECFGAREC                          ((uint32_t)0x00040000)
#define TSI721_PHYLCFG0_TLW                               ((uint32_t)0x00380000)
#define TSI721_PHYLCFG0_SLANEREV                          ((uint32_t)0x00400000)
#define TSI721_PHYLCFG0_FLANEREV                          ((uint32_t)0x00800000)
#define TSI721_PHYLCFG0_RDETECT                           ((uint32_t)0x30000000)

/* TSI721_PHYLCFG1 : Register Bits Masks Definitions */
#define TSI721_PHYLCFG1_LNPOLOR                           ((uint32_t)0x0000000f)
#define TSI721_PHYLCFG1_LNPOLOREN                         ((uint32_t)0x00000100)
#define TSI721_PHYLCFG1_NFTS_TOC                          ((uint32_t)0x00ff0000)
#define TSI721_PHYLCFG1_TXEIDL                            ((uint32_t)0x03000000)
#define TSI721_PHYLCFG1_L0S_RXEIDL                        ((uint32_t)0x0c000000)
#define TSI721_PHYLCFG1_L0S_RXSKP                         ((uint32_t)0x30000000)
#define TSI721_PHYLCFG1_TX_FULL_SKP                       ((uint32_t)0x40000000)

/* TSI721_PHYLSTS0 : Register Bits Masks Definitions */
#define TSI721_PHYLSTS0_STPSDP                            ((uint32_t)0x00000001)
#define TSI721_PHYLSTS0_PADERR                            ((uint32_t)0x00000002)
#define TSI721_PHYLSTS0_SEOPERR                           ((uint32_t)0x00000004)
#define TSI721_PHYLSTS0_EOPPERR                           ((uint32_t)0x00000008)
#define TSI721_PHYLSTS0_DSOPERR                           ((uint32_t)0x00000010)
#define TSI721_PHYLSTS0_SOPEOPERR                         ((uint32_t)0x00000020)
#define TSI721_PHYLSTS0_SOPLERR                           ((uint32_t)0x00000040)
#define TSI721_PHYLSTS0_RECDET                            ((uint32_t)0x00000200)
#define TSI721_PHYLSTS0_RECCON                            ((uint32_t)0x00000400)
#define TSI721_PHYLSTS0_L0SREC                            ((uint32_t)0x00000800)
#define TSI721_PHYLSTS0_ILW                               ((uint32_t)0x0000c000)
#define TSI721_PHYLSTS0_LPWUC                             ((uint32_t)0x00010000)
#define TSI721_PHYLSTS0_LNPOLORSTS                        ((uint32_t)0xff000000)

/* TSI721_PHYLSTS1 : Register Bits Masks Definitions */
#define TSI721_PHYLSTS1_TRAINDE                           ((uint32_t)0x00000002)
#define TSI721_PHYLSTS1_RFTSOS                            ((uint32_t)0x00000004)
#define TSI721_PHYLSTS1_RIDLOS                            ((uint32_t)0x00000008)
#define TSI721_PHYLSTS1_RTS2OS                            ((uint32_t)0x00000010)
#define TSI721_PHYLSTS1_RTS1OS                            ((uint32_t)0x00000020)
#define TSI721_PHYLSTS1_RSKPOS                            ((uint32_t)0x00000040)
#define TSI721_PHYLSTS1_RXFRERR                           ((uint32_t)0x00010000)
#define TSI721_PHYLSTS1_RXDISPERR                         ((uint32_t)0x00020000)
#define TSI721_PHYLSTS1_RXUFERR                           ((uint32_t)0x00040000)
#define TSI721_PHYLSTS1_RXOFERR                           ((uint32_t)0x00080000)
#define TSI721_PHYLSTS1_RX8B10BERR                        ((uint32_t)0x00100000)

/* TSI721_PHYLSTATE0 : Register Bits Masks Definitions */
#define TSI721_PHYLSTATE0_LTSSMSTATE                      ((uint32_t)0x0000001f)
#define TSI721_PHYLSTATE0_TXLSTATE                        ((uint32_t)0x000000e0)
#define TSI721_PHYLSTATE0_RXLSTATE                        ((uint32_t)0x00000700)
#define TSI721_PHYLSTATE0_SRMBLSTAT                       ((uint32_t)0x00000800)
#define TSI721_PHYLSTATE0_LPNFTS                          ((uint32_t)0x000ff000)
#define TSI721_PHYLSTATE0_LNKNUM                          ((uint32_t)0x1ff00000)
#define TSI721_PHYLSTATE0_LANEREV                         ((uint32_t)0x20000000)
#define TSI721_PHYLSTATE0_FLRET                           ((uint32_t)0x80000000)

/* TSI721_PHYLTSSMSTS0 : Register Bits Masks Definitions */
#define TSI721_PHYLTSSMSTS0_XMIT_EIOS                     ((uint32_t)0x00000001)
#define TSI721_PHYLTSSMSTS0_TMOUT_1MS                     ((uint32_t)0x00000002)
#define TSI721_PHYLTSSMSTS0_DQUIET                        ((uint32_t)0x00000004)
#define TSI721_PHYLTSSMSTS0_DACTIVE                       ((uint32_t)0x00000008)
#define TSI721_PHYLTSSMSTS0_PACTIVE                       ((uint32_t)0x00000010)
#define TSI721_PHYLTSSMSTS0_PCOMP                         ((uint32_t)0x00000020)
#define TSI721_PHYLTSSMSTS0_PCONFIG                       ((uint32_t)0x00000040)
#define TSI721_PHYLTSSMSTS0_CLWSTART                      ((uint32_t)0x00000100)
#define TSI721_PHYLTSSMSTS0_CLWACCEPT                     ((uint32_t)0x00000200)
#define TSI721_PHYLTSSMSTS0_CLNWAIT                       ((uint32_t)0x00000400)
#define TSI721_PHYLTSSMSTS0_CLNACCEPT                     ((uint32_t)0x00000800)
#define TSI721_PHYLTSSMSTS0_CCOMPLETE                     ((uint32_t)0x00001000)
#define TSI721_PHYLTSSMSTS0_CIDLE                         ((uint32_t)0x00002000)
#define TSI721_PHYLTSSMSTS0_TOUPCFG                       ((uint32_t)0x00008000)
#define TSI721_PHYLTSSMSTS0_RECRCVLOCK                    ((uint32_t)0x00010000)
#define TSI721_PHYLTSSMSTS0_RECRCVCFG                     ((uint32_t)0x00020000)
#define TSI721_PHYLTSSMSTS0_RECIDLE                       ((uint32_t)0x00040000)
#define TSI721_PHYLTSSMSTS0_RECSPEED                      ((uint32_t)0x00080000)
#define TSI721_PHYLTSSMSTS0_L0                            ((uint32_t)0x00100000)
#define TSI721_PHYLTSSMSTS0_L0S                           ((uint32_t)0x00200000)
#define TSI721_PHYLTSSMSTS0_L1ENTRY                       ((uint32_t)0x00400000)
#define TSI721_PHYLTSSMSTS0_L1IDLE                        ((uint32_t)0x00800000)
#define TSI721_PHYLTSSMSTS0_L2IDLE                        ((uint32_t)0x01000000)
#define TSI721_PHYLTSSMSTS0_L2XMITWAKE                    ((uint32_t)0x02000000)
#define TSI721_PHYLTSSMSTS0_DISABLED                      ((uint32_t)0x04000000)
#define TSI721_PHYLTSSMSTS0_HOTRESET                      ((uint32_t)0x08000000)
#define TSI721_PHYLTSSMSTS0_LBENTRY                       ((uint32_t)0x10000000)
#define TSI721_PHYLTSSMSTS0_LBACTIVE                      ((uint32_t)0x20000000)
#define TSI721_PHYLTSSMSTS0_LBEXIT                        ((uint32_t)0x40000000)
#define TSI721_PHYLTSSMSTS0_IDT_TM                        ((uint32_t)0x80000000)

/* TSI721_PHYLTSSMSTS1 : Register Bits Masks Definitions */
#define TSI721_PHYLTSSMSTS1_TXACTIVE                      ((uint32_t)0x00000001)
#define TSI721_PHYLTSSMSTS1_TXL0SENTRY                    ((uint32_t)0x00000002)
#define TSI721_PHYLTSSMSTS1_TXL0SIDLE                     ((uint32_t)0x00000004)
#define TSI721_PHYLTSSMSTS1_TXL0SFTS                      ((uint32_t)0x00000008)
#define TSI721_PHYLTSSMSTS1_RXACTIVE                      ((uint32_t)0x00000010)
#define TSI721_PHYLTSSMSTS1_RXL0SENTRY                    ((uint32_t)0x00000020)
#define TSI721_PHYLTSSMSTS1_RXL0SIDLE                     ((uint32_t)0x00000040)
#define TSI721_PHYLTSSMSTS1_RXL0SFTS                      ((uint32_t)0x00000080)

/* TSI721_PHYCNT0 : Register Bits Masks Definitions */
#define TSI721_PHYCNT0_COUNT                              ((uint32_t)0xffffffff)

/* TSI721_PHYCNT1 : Register Bits Masks Definitions */
#define TSI721_PHYCNT1_COUNT                              ((uint32_t)0xffffffff)

/* TSI721_PHYCNTCFG : Register Bits Masks Definitions */
#define TSI721_PHYCNTCFG_PHYCNT0SEL                       ((uint32_t)0x0000007f)
#define TSI721_PHYCNTCFG_PHYCNT1SEL                       ((uint32_t)0x00007f00)
#define TSI721_PHYCNTCFG_LANESEL                          ((uint32_t)0x001f0000)

/* TSI721_PHYRECEL : Register Bits Masks Definitions */
#define TSI721_PHYRECEL_RCOUNT                            ((uint32_t)0x000000ff)
#define TSI721_PHYRECEL_LRET                              ((uint32_t)0x00000100)
#define TSI721_PHYRECEL_LDIS                              ((uint32_t)0x00000200)
#define TSI721_PHYRECEL_DDL                               ((uint32_t)0x00000400)
#define TSI721_PHYRECEL_DSKERR                            ((uint32_t)0x00000800)
#define TSI721_PHYRECEL_ILSC                              ((uint32_t)0x00001000)
#define TSI721_PHYRECEL_OLSC                              ((uint32_t)0x00004000)
#define TSI721_PHYRECEL_TSL0                              ((uint32_t)0x00008000)
#define TSI721_PHYRECEL_EIDLL0                            ((uint32_t)0x00010000)
#define TSI721_PHYRECEL_NFTSTO                            ((uint32_t)0x00020000)
#define TSI721_PHYRECEL_HOTRST                            ((uint32_t)0x00040000)
#define TSI721_PHYRECEL_CITO                              ((uint32_t)0x00080000)
#define TSI721_PHYRECEL_L1EXIT                            ((uint32_t)0x00100000)
#define TSI721_PHYRECEL_TM                                ((uint32_t)0x00200000)
#define TSI721_PHYRECEL_FC                                ((uint32_t)0x0f000000)
#define TSI721_PHYRECEL_ENLOG                             ((uint32_t)0x80000000)

/* TSI721_PHYPRBS : Register Bits Masks Definitions */
#define TSI721_PHYPRBS_SEED                               ((uint32_t)0x0000ffff)

/* TSI721_DLCTL1 : Register Bits Masks Definitions */
#define TSI721_DLCTL1_RPTIMEOUT                           ((uint32_t)0x00007fff)
#define TSI721_DLCTL1_RPTIMEOUTO                          ((uint32_t)0x00008000)
#define TSI721_DLCTL1_ANTIMEOUT                           ((uint32_t)0x7fff0000)
#define TSI721_DLCTL1_ANTIMEOUTO                          ((uint32_t)0x80000000)

/* TSI721_DLCTL2 : Register Bits Masks Definitions */
#define TSI721_DLCTL2_INITFCVALVC                         ((uint32_t)0x00003fff)
#define TSI721_DLCTL2_INITFCTOVC                          ((uint32_t)0x00004000)
#define TSI721_DLCTL2_DISCRCCHK                           ((uint32_t)0x80000000)

/* TSI721_DLCTL3 : Register Bits Masks Definitions */
#define TSI721_DLCTL3_DLLPRXTO                            ((uint32_t)0x0001ffff)
#define TSI721_DLCTL3_DLLPRXTE                            ((uint32_t)0x00100000)

/* TSI721_DLSTS : Register Bits Masks Definitions */
#define TSI721_DLSTS_DLFSM                                ((uint32_t)0x00000003)
#define TSI721_DLSTS_RXFERR                               ((uint32_t)0x00000010)
#define TSI721_DLSTS_RXPROTERR                            ((uint32_t)0x00000080)
#define TSI721_DLSTS_DLBUFOVRFL                           ((uint32_t)0x00000100)

/* TSI721_DLRXSTS : Register Bits Masks Definitions */
#define TSI721_DLRXSTS_TLP                                ((uint32_t)0x00000001)
#define TSI721_DLRXSTS_TLPCRCERR                          ((uint32_t)0x00000002)
#define TSI721_DLRXSTS_DLLP                               ((uint32_t)0x00000004)
#define TSI721_DLRXSTS_DLLPCRCERR                         ((uint32_t)0x00000008)
#define TSI721_DLRXSTS_NACKDLLP                           ((uint32_t)0x00000010)
#define TSI721_DLRXSTS_ACKDLLP                            ((uint32_t)0x00000020)
#define TSI721_DLRXSTS_IFCDLLP1                           ((uint32_t)0x00000040)
#define TSI721_DLRXSTS_IFCDLLP2                           ((uint32_t)0x00000100)
#define TSI721_DLRXSTS_UFCDLLP                            ((uint32_t)0x00000400)
#define TSI721_DLRXSTS_PMDLLP                             ((uint32_t)0x00001000)
#define TSI721_DLRXSTS_ROSEQ                              ((uint32_t)0x00002000)
#define TSI721_DLRXSTS_RBEDB                              ((uint32_t)0x00004000)
#define TSI721_DLRXSTS_RDUPTLP                            ((uint32_t)0x00008000)
#define TSI721_DLRXSTS_RTLPNULL                           ((uint32_t)0x00020000)
#define TSI721_DLRXSTS_DLLPRXTO                           ((uint32_t)0x00040000)
#define TSI721_DLRXSTS_RXVDDLLP                           ((uint32_t)0x00080000)
#define TSI721_DLRXSTS_RXUDLLP                            ((uint32_t)0x00100000)
#define TSI721_DLRXSTS_RXTLPLERR                          ((uint32_t)0x00200000)

/* TSI721_DLTXSTS : Register Bits Masks Definitions */
#define TSI721_DLTXSTS_TLP                                ((uint32_t)0x00000001)
#define TSI721_DLTXSTS_TXNRPTLP                           ((uint32_t)0x00000002)
#define TSI721_DLTXSTS_TXRPTLP                            ((uint32_t)0x00000004)
#define TSI721_DLTXSTS_TXTLPNULL                          ((uint32_t)0x00000008)
#define TSI721_DLTXSTS_DLLP                               ((uint32_t)0x00000040)
#define TSI721_DLTXSTS_NACKDLLP                           ((uint32_t)0x00000080)
#define TSI721_DLTXSTS_ACKDLLP                            ((uint32_t)0x00000100)
#define TSI721_DLTXSTS_IFCDLLP1                           ((uint32_t)0x00000200)
#define TSI721_DLTXSTS_IFCDLLP2                           ((uint32_t)0x00000800)
#define TSI721_DLTXSTS_UFCDLLP                            ((uint32_t)0x00002000)
#define TSI721_DLTXSTS_PMDLLP                             ((uint32_t)0x00008000)
#define TSI721_DLTXSTS_ANTIMOUT                           ((uint32_t)0x00010000)
#define TSI721_DLTXSTS_REPLAYTO                           ((uint32_t)0x00020000)
#define TSI721_DLTXSTS_REPLAYEVNT                         ((uint32_t)0x00040000)
#define TSI721_DLTXSTS_RPNUMRO                            ((uint32_t)0x00200000)
#define TSI721_DLTXSTS_VC0INITFCTO                        ((uint32_t)0x01000000)

/* TSI721_DLCNT0 : Register Bits Masks Definitions */
#define TSI721_DLCNT0_COUNT                               ((uint32_t)0xffffffff)

/* TSI721_DLCNT1 : Register Bits Masks Definitions */
#define TSI721_DLCNT1_COUNT                               ((uint32_t)0xffffffff)

/* TSI721_DLCNTCFG : Register Bits Masks Definitions */
#define TSI721_DLCNTCFG_DLCNT0SEL                         ((uint32_t)0x0000001f)
#define TSI721_DLCNTCFG_DLCNT1SEL                         ((uint32_t)0x00001f00)

/* TSI721_TLSTSE : Register Bits Masks Definitions */
#define TSI721_TLSTSE_RUR                                 ((uint32_t)0x00000001)
#define TSI721_TLSTSE_MALFORMED                           ((uint32_t)0x00000004)
#define TSI721_TLSTSE_NULLIFIED                           ((uint32_t)0x00000008)
#define TSI721_TLSTSE_RO                                  ((uint32_t)0x00000010)
#define TSI721_TLSTSE_RCVTLP                              ((uint32_t)0x00010000)
#define TSI721_TLSTSE_RTLPCPE                             ((uint32_t)0x00040000)
#define TSI721_TLSTSE_RTLPMPE                             ((uint32_t)0x00080000)
#define TSI721_TLSTSE_RTLPIOPE                            ((uint32_t)0x00100000)
#define TSI721_TLSTSE_IETLPME                             ((uint32_t)0x00200000)
#define TSI721_TLSTSE_RTLPME                              ((uint32_t)0x00400000)

/* TSI721_TLCTL : Register Bits Masks Definitions */
#define TSI721_TLCTL_FCUTIMER                             ((uint32_t)0x000003ff)
#define TSI721_TLCTL_FCUTIMERO                            ((uint32_t)0x00000400)
#define TSI721_TLCTL_SEQTAG                               ((uint32_t)0x00000800)

/* TSI721_TLCNT0 : Register Bits Masks Definitions */
#define TSI721_TLCNT0_COUNT                               ((uint32_t)0xffffffff)

/* TSI721_TLCNT1 : Register Bits Masks Definitions */
#define TSI721_TLCNT1_COUNT                               ((uint32_t)0xffffffff)

/* TSI721_TLCNTCFG : Register Bits Masks Definitions */
#define TSI721_TLCNTCFG_TLCNT0SEL                         ((uint32_t)0x0000001f)
#define TSI721_TLCNTCFG_TLCNT1SEL                         ((uint32_t)0x000003e0)
#define TSI721_TLCNTCFG_FUNC                              ((uint32_t)0x00070000)
#define TSI721_TLCNTCFG_DEV                               ((uint32_t)0x00f80000)
#define TSI721_TLCNTCFG_BUS                               ((uint32_t)0xff000000)

/* TSI721_INTSTS : Register Bits Masks Definitions */
#define TSI721_INTSTS_INTA                                ((uint32_t)0x00000001)
#define TSI721_INTSTS_INTB                                ((uint32_t)0x00000002)
#define TSI721_INTSTS_INTC                                ((uint32_t)0x00000004)
#define TSI721_INTSTS_INTD                                ((uint32_t)0x00000008)

/* TSI721_PMPC0 : Register Bits Masks Definitions */
#define TSI721_PMPC0_L0ET                                 ((uint32_t)0x0fff0000)
#define TSI721_PMPC0_L0SASPMD                             ((uint32_t)0x10000000)
#define TSI721_PMPC0_L1ASPMD                              ((uint32_t)0x20000000)
#define TSI721_PMPC0_D3HOTL1D                             ((uint32_t)0x40000000)

/* TSI721_PMPC1 : Register Bits Masks Definitions */
#define TSI721_PMPC1_PMCS                                 ((uint32_t)0x0000003f)
#define TSI721_PMPC1_EXITL1                               ((uint32_t)0x00000100)
#define TSI721_PMPC1_ENTRLTR                              ((uint32_t)0x00000200)
#define TSI721_PMPC1_ENTRLTA                              ((uint32_t)0x00000400)
#define TSI721_PMPC1_ENTRL1T                              ((uint32_t)0x00000800)
#define TSI721_PMPC1_ENTRL1C                              ((uint32_t)0x00001000)
#define TSI721_PMPC1_ENTRL1P                              ((uint32_t)0x00002000)
#define TSI721_PMPC1_EXITL0S                              ((uint32_t)0x00004000)
#define TSI721_PMPC1_ENTRL0S                              ((uint32_t)0x00008000)

/* TSI721_FCVC0PTCC : Register Bits Masks Definitions */
#define TSI721_FCVC0PTCC_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0PTCC_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0NPCC : Register Bits Masks Definitions */
#define TSI721_FCVC0NPCC_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0NPCC_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0CPCC : Register Bits Masks Definitions */
#define TSI721_FCVC0CPCC_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0CPCC_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0PTCL : Register Bits Masks Definitions */
#define TSI721_FCVC0PTCL_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0PTCL_HDRFCCC                          ((uint32_t)0x003fc000)
#define TSI721_FCVC0PTCL_INFDAT                           ((uint32_t)0x20000000)
#define TSI721_FCVC0PTCL_INFHDR                           ((uint32_t)0x40000000)
#define TSI721_FCVC0PTCL_VALID                            ((uint32_t)0x80000000)

/* TSI721_FCVC0NPCL : Register Bits Masks Definitions */
#define TSI721_FCVC0NPCL_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0NPCL_HDRFCCC                          ((uint32_t)0x003fc000)
#define TSI721_FCVC0NPCL_INFDAT                           ((uint32_t)0x20000000)
#define TSI721_FCVC0NPCL_INFHDR                           ((uint32_t)0x40000000)
#define TSI721_FCVC0NPCL_VALID                            ((uint32_t)0x80000000)

/* TSI721_FCVC0CPCL : Register Bits Masks Definitions */
#define TSI721_FCVC0CPCL_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0CPCL_HDRFCCC                          ((uint32_t)0x003fc000)
#define TSI721_FCVC0CPCL_INFDAT                           ((uint32_t)0x20000000)
#define TSI721_FCVC0CPCL_INFHDR                           ((uint32_t)0x40000000)
#define TSI721_FCVC0CPCL_VALID                            ((uint32_t)0x80000000)

/* TSI721_FCVC0PTCA : Register Bits Masks Definitions */
#define TSI721_FCVC0PTCA_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0PTCA_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0NPCA : Register Bits Masks Definitions */
#define TSI721_FCVC0NPCA_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0NPCA_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0CPCA : Register Bits Masks Definitions */
#define TSI721_FCVC0CPCA_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0CPCA_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0PTCR : Register Bits Masks Definitions */
#define TSI721_FCVC0PTCR_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0PTCR_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0NPCR : Register Bits Masks Definitions */
#define TSI721_FCVC0NPCR_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0NPCR_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_FCVC0CPCR : Register Bits Masks Definitions */
#define TSI721_FCVC0CPCR_DATAFCCC                         ((uint32_t)0x00000fff)
#define TSI721_FCVC0CPCR_HDRFCCC                          ((uint32_t)0x003fc000)

/* TSI721_EFBTC : Register Bits Masks Definitions */
#define TSI721_EFBTC_VC0IPTT                              ((uint32_t)0x00000001)
#define TSI721_EFBTC_VC0INPT                              ((uint32_t)0x00000002)
#define TSI721_EFBTC_VC0ICPT                              ((uint32_t)0x00000004)

/* TSI721_IFBCNT0 : Register Bits Masks Definitions */
#define TSI721_IFBCNT0_COUNT                              ((uint32_t)0xffffffff)

/* TSI721_IFBCNT1 : Register Bits Masks Definitions */
#define TSI721_IFBCNT1_COUNT                              ((uint32_t)0xffffffff)

/* TSI721_IFBCNTCFG : Register Bits Masks Definitions */
#define TSI721_IFBCNTCFG_IFBCNT0SEL                       ((uint32_t)0x0000001f)
#define TSI721_IFBCNTCFG_IFBCNT1SEL                       ((uint32_t)0x000003e0)

/* TSI721_EFBCNT0 : Register Bits Masks Definitions */
#define TSI721_EFBCNT0_COUNT                              ((uint32_t)0xffffffff)

/* TSI721_EFBCNT1 : Register Bits Masks Definitions */
#define TSI721_EFBCNT1_COUNT                              ((uint32_t)0xffffffff)

/* TSI721_EFBCNTCFG : Register Bits Masks Definitions */
#define TSI721_EFBCNTCFG_EFBCNT0SEL                       ((uint32_t)0x0000001f)
#define TSI721_EFBCNTCFG_EFBCNT1SEL                       ((uint32_t)0x000003e0)

/* TSI721_UEEM : Register Bits Masks Definitions */
#define TSI721_UEEM_DLPERR                                ((uint32_t)0x00000010)
#define TSI721_UEEM_POISONED                              ((uint32_t)0x00001000)
#define TSI721_UEEM_COMPTO                                ((uint32_t)0x00004000)
#define TSI721_UEEM_CABORT                                ((uint32_t)0x00008000)
#define TSI721_UEEM_UECOMP                                ((uint32_t)0x00010000)
#define TSI721_UEEM_RCVOVR                                ((uint32_t)0x00020000)
#define TSI721_UEEM_MALFORMED                             ((uint32_t)0x00040000)
#define TSI721_UEEM_ECRC                                  ((uint32_t)0x00080000)
#define TSI721_UEEM_UR                                    ((uint32_t)0x00100000)
#define TSI721_UEEM_UIE                                   ((uint32_t)0x00400000)
#define TSI721_UEEM_ADVISORYNF                            ((uint32_t)0x80000000)

/* TSI721_CEEM : Register Bits Masks Definitions */
#define TSI721_CEEM_RCVERR                                ((uint32_t)0x00000001)
#define TSI721_CEEM_BADTLP                                ((uint32_t)0x00000040)
#define TSI721_CEEM_BADDLLP                               ((uint32_t)0x00000080)
#define TSI721_CEEM_RPLYROVR                              ((uint32_t)0x00000100)
#define TSI721_CEEM_RPLYTO                                ((uint32_t)0x00001000)
#define TSI721_CEEM_CIE                                   ((uint32_t)0x00004000)
#define TSI721_CEEM_HLO                                   ((uint32_t)0x00008000)

/* TSI721_STMCTL : Register Bits Masks Definitions */
#define TSI721_STMCTL_CMD                                 ((uint32_t)0x00000007)
#define TSI721_STMCTL_SPEED                               ((uint32_t)0x00000008)

/* TSI721_STMSTS : Register Bits Masks Definitions */
#define TSI721_STMSTS_CC                                  ((uint32_t)0x00000001)
#define TSI721_STMSTS_PSTATE                              ((uint32_t)0x00000006)

/* TSI721_STMTCTL : Register Bits Masks Definitions */
#define TSI721_STMTCTL_TSEL                               ((uint32_t)0x0000000f)
#define TSI721_STMTCTL_TSYNCP                             ((uint32_t)0x00000030)

/* TSI721_STMTSTS : Register Bits Masks Definitions */
#define TSI721_STMTSTS_SYNC                               ((uint32_t)0x0000000f)
#define TSI721_STMTSTS_TR                                 ((uint32_t)0x80000000)

/* TSI721_STMECNT0 : Register Bits Masks Definitions */
#define TSI721_STMECNT0_COUNT                             ((uint32_t)0x000003ff)
#define TSI721_STMECNT0_OVR                               ((uint32_t)0x00000400)
#define TSI721_STMECNT0_ERR_DET                           ((uint32_t)0x00000800)
#define TSI721_STMECNT0_ERR_INJ                           ((uint32_t)0x00010000)

/* TSI721_STMECNT1 : Register Bits Masks Definitions */
#define TSI721_STMECNT1_COUNT                             ((uint32_t)0x000003ff)
#define TSI721_STMECNT1_OVR                               ((uint32_t)0x00000400)
#define TSI721_STMECNT1_ERR_DET                           ((uint32_t)0x00000800)
#define TSI721_STMECNT1_ERR_INJ                           ((uint32_t)0x00010000)

/* TSI721_STMECNT2 : Register Bits Masks Definitions */
#define TSI721_STMECNT2_COUNT                             ((uint32_t)0x000003ff)
#define TSI721_STMECNT2_OVR                               ((uint32_t)0x00000400)
#define TSI721_STMECNT2_ERR_DET                           ((uint32_t)0x00000800)
#define TSI721_STMECNT2_ERR_INJ                           ((uint32_t)0x00010000)

/* TSI721_STMECNT3 : Register Bits Masks Definitions */
#define TSI721_STMECNT3_COUNT                             ((uint32_t)0x000003ff)
#define TSI721_STMECNT3_OVR                               ((uint32_t)0x00000400)
#define TSI721_STMECNT3_ERR_DET                           ((uint32_t)0x00000800)
#define TSI721_STMECNT3_ERR_INJ                           ((uint32_t)0x00010000)

/* TSI721_ALLCS : Register Bits Masks Definitions */
#define TSI721_ALLCS_ALLME                                ((uint32_t)0x00000001)
#define TSI721_ALLCS_ALLS                                 ((uint32_t)0x80000000)

/* TSI721_IFBVC0PTCFG : Register Bits Masks Definitions */
#define TSI721_IFBVC0PTCFG_PTHDR                          ((uint32_t)0x000000ff)
#define TSI721_IFBVC0PTCFG_PTDATA                         ((uint32_t)0x03ff0000)

/* TSI721_IFBVC0NPCFG : Register Bits Masks Definitions */
#define TSI721_IFBVC0NPCFG_NPHDR                          ((uint32_t)0x000000ff)
#define TSI721_IFBVC0NPCFG_NPDATA                         ((uint32_t)0x03ff0000)

/* TSI721_IFBVC0CPCFG : Register Bits Masks Definitions */
#define TSI721_IFBVC0CPCFG_CPHDR                          ((uint32_t)0x000000ff)
#define TSI721_IFBVC0CPCFG_CPDATA                         ((uint32_t)0x03ff0000)

/* TSI721_IFCSTS : Register Bits Masks Definitions */
#define TSI721_IFCSTS_VC0PTHO                             ((uint32_t)0x00000001)
#define TSI721_IFCSTS_VC0NPHO                             ((uint32_t)0x00000002)
#define TSI721_IFCSTS_VC0CPHO                             ((uint32_t)0x00000004)
#define TSI721_IFCSTS_VC0PTDO                             ((uint32_t)0x00000008)
#define TSI721_IFCSTS_VC0NPDO                             ((uint32_t)0x00000010)
#define TSI721_IFCSTS_VC0CPDO                             ((uint32_t)0x00000020)

/* TSI721_EFBVC0PTSTS : Register Bits Masks Definitions */
#define TSI721_EFBVC0PTSTS_HDR                            ((uint32_t)0x000000ff)
#define TSI721_EFBVC0PTSTS_DATA                           ((uint32_t)0x07ff0000)

/* TSI721_EFBVC0NPSTS : Register Bits Masks Definitions */
#define TSI721_EFBVC0NPSTS_HDR                            ((uint32_t)0x000000ff)
#define TSI721_EFBVC0NPSTS_DATA                           ((uint32_t)0x07ff0000)

/* TSI721_EFBVC0CPSTS : Register Bits Masks Definitions */
#define TSI721_EFBVC0CPSTS_HDR                            ((uint32_t)0x000000ff)
#define TSI721_EFBVC0CPSTS_DATA                           ((uint32_t)0x07ff0000)

/* TSI721_EFBRBSTS : Register Bits Masks Definitions */
#define TSI721_EFBRBSTS_FULL                              ((uint32_t)0x00000100)


/******************************************************/
/* TSI721 : S-RIO Register address offset definitions */
/******************************************************/

#define TSI721_MAX_PORTS 1
#define TSI721_MAX_LANES 4

#define TSI721_DEV_ID                                    ((uint32_t)0x00000000)
#define TSI721_DEV_INFO                                  ((uint32_t)0x00000004)
#define TSI721_ASBLY_ID                                  ((uint32_t)0x00000008)
#define TSI721_ASBLY_INFO                                ((uint32_t)0x0000000c)
#define TSI721_PE_FEAT                                   ((uint32_t)0x00000010)
#define TSI721_SRC_OP                                    ((uint32_t)0x00000018)
#define TSI721_DEST_OP                                   ((uint32_t)0x0000001c)
#define TSI721_SR_XADDR                                  ((uint32_t)0x0000004c)
#define TSI721_BASE_ID                                   ((uint32_t)0x00000060)
#define TSI721_HOST_BASE_ID_LOCK                         ((uint32_t)0x00000068)
#define TSI721_COMP_TAG                                  ((uint32_t)0x0000006c)
#define TSI721_SP_MB_HEAD                                ((uint32_t)0x00000100)
#define TSI721_SP_LT_CTL                                 ((uint32_t)0x00000120)
#define TSI721_SR_RSP_TO                                 ((uint32_t)0x00000124)
#define TSI721_SP_GEN_CTL                                ((uint32_t)0x0000013c)
#define TSI721_SP_LM_REQ                                 ((uint32_t)0x00000140)
#define TSI721_SP_LM_RESP                                ((uint32_t)0x00000144)
#define TSI721_SP_ACKID_STAT                             ((uint32_t)0x00000148)
#define TSI721_SP_CTL2                                   ((uint32_t)0x00000154)
#define TSI721_SP_ERR_STAT                               ((uint32_t)0x00000158)
#define TSI721_SP_CTL                                    ((uint32_t)0x0000015c)
#define TSI721_ERR_RPT_BH                                ((uint32_t)0x00001000)
#define TSI721_ERR_DET                                   ((uint32_t)0x00001008)
#define TSI721_ERR_EN                                    ((uint32_t)0x0000100c)
#define TSI721_H_ADDR_CAPT                               ((uint32_t)0x00001010)
#define TSI721_ADDR_CAPT                                 ((uint32_t)0x00001014)
#define TSI721_ID_CAPT                                   ((uint32_t)0x00001018)
#define TSI721_CTRL_CAPT                                 ((uint32_t)0x0000101c)
#define TSI721_PW_TGT_ID                                 ((uint32_t)0x00001028)
#define TSI721_SP_ERR_DET                                ((uint32_t)0x00001040)
#define TSI721_SP_RATE_EN                                ((uint32_t)0x00001044)
#define TSI721_SP_ERR_ATTR_CAPT                          ((uint32_t)0x00001048)
#define TSI721_SP_ERR_CAPT_0                             ((uint32_t)0x0000104c)
#define TSI721_SP_ERR_CAPT_1                             ((uint32_t)0x00001050)
#define TSI721_SP_ERR_CAPT_2                             ((uint32_t)0x00001054)
#define TSI721_SP_ERR_CAPT_3                             ((uint32_t)0x00001058)
#define TSI721_SP_ERR_RATE                               ((uint32_t)0x00001068)
#define TSI721_SP_ERR_THRESH                             ((uint32_t)0x0000106c)
#define TSI721_PER_LANE_BH                               ((uint32_t)0x00003000)
#define TSI721_LANEX_STAT0(X)                  ((uint32_t)(0x3010 + 0x020*(X)))
#define TSI721_LANEX_STAT1(X)                  ((uint32_t)(0x3014 + 0x020*(X)))
#define TSI721_PLM_BH                                    ((uint32_t)0x00010000)
#define TSI721_PLM_IMP_SPEC_CTL                          ((uint32_t)0x00010080)
#define TSI721_PLM_STATUS                                ((uint32_t)0x00010090)
#define TSI721_PLM_INT_ENABLE                            ((uint32_t)0x00010094)
#define TSI721_PLM_PW_ENABLE                             ((uint32_t)0x00010098)
#define TSI721_PLM_EVENT_GEN                             ((uint32_t)0x0001009c)
#define TSI721_PLM_ALL_INT_EN                            ((uint32_t)0x000100a0)
#define TSI721_PLM_ALL_PW_EN                             ((uint32_t)0x000100a4)
#define TSI721_PLM_DISCOVERY_TIMER                       ((uint32_t)0x000100b4)
#define TSI721_PLM_SILENCE_TIMER                         ((uint32_t)0x000100b8)
#define TSI721_PLM_VMIN_EXP                              ((uint32_t)0x000100bc)
#define TSI721_PLM_POL_CTL                               ((uint32_t)0x000100c0)
#define TSI721_PLM_DENIAL_CTL                            ((uint32_t)0x000100c8)
#define TSI721_PLM_RCVD_MECS                             ((uint32_t)0x000100d0)
#define TSI721_PLM_MECS_FWD                              ((uint32_t)0x000100d8)
#define TSI721_PLM_LONG_CS_TX1                           ((uint32_t)0x000100e0)
#define TSI721_PLM_LONG_CS_TX2                           ((uint32_t)0x000100e4)
#define TSI721_TLM_BH                                    ((uint32_t)0x00010300)
#define TSI721_TLM_SP_CONTROL                            ((uint32_t)0x00010380)
#define TSI721_TLM_SP_STATUS                             ((uint32_t)0x00010390)
#define TSI721_TLM_SP_INT_ENABLE                         ((uint32_t)0x00010394)
#define TSI721_TLM_SP_PW_ENABLE                          ((uint32_t)0x00010398)
#define TSI721_TLM_SP_EVENT_GEN                          ((uint32_t)0x0001039c)
#define TSI721_TLM_SP_BRR_CTLX(X)             ((uint32_t)(0x103a0 + 0x010*(X)))
#define TSI721_TLM_SP_BRR_PATTERN_MATCHX(X)   ((uint32_t)(0x103a4 + 0x010*(X)))
#define TSI721_TLM_SP_FTYPE_FILTER_CTL                   ((uint32_t)0x000103e0)
#define TSI721_PBM_BH                                    ((uint32_t)0x00010600)
#define TSI721_PBM_SP_CONTROL                            ((uint32_t)0x00010680)
#define TSI721_PBM_SP_STATUS                             ((uint32_t)0x00010690)
#define TSI721_PBM_SP_INT_ENABLE                         ((uint32_t)0x00010694)
#define TSI721_PBM_SP_PW_ENABLE                          ((uint32_t)0x00010698)
#define TSI721_PBM_SP_EVENT_GEN                          ((uint32_t)0x0001069c)
#define TSI721_PBM_SP_IG_WATERMARK0                      ((uint32_t)0x000106b0)
#define TSI721_PBM_SP_IG_WATERMARK1                      ((uint32_t)0x000106b4)
#define TSI721_PBM_SP_IG_WATERMARK2                      ((uint32_t)0x000106b8)
#define TSI721_PBM_SP_IG_WATERMARK3                      ((uint32_t)0x000106bc)
#define TSI721_EM_BH                                     ((uint32_t)0x00010900)
#define TSI721_EM_INT_STAT                               ((uint32_t)0x00010910)
#define TSI721_EM_INT_ENABLE                             ((uint32_t)0x00010914)
#define TSI721_EM_INT_PORT_STAT                          ((uint32_t)0x00010918)
#define TSI721_EM_PW_STAT                                ((uint32_t)0x00010920)
#define TSI721_EM_PW_ENABLE                              ((uint32_t)0x00010924)
#define TSI721_EM_PW_PORT_STAT                           ((uint32_t)0x00010928)
#define TSI721_EM_DEV_INT_EN                             ((uint32_t)0x00010930)
#define TSI721_EM_DEV_PW_EN                              ((uint32_t)0x00010934)
#define TSI721_EM_MECS_STAT                              ((uint32_t)0x0001093c)
#define TSI721_EM_MECS_INT_EN                            ((uint32_t)0x00010940)
#define TSI721_EM_MECS_CAP_EN                            ((uint32_t)0x00010944)
#define TSI721_EM_MECS_TRIG_EN                           ((uint32_t)0x00010948)
#define TSI721_EM_MECS_REQ                               ((uint32_t)0x0001094c)
#define TSI721_EM_MECS_PORT_STAT                         ((uint32_t)0x00010950)
#define TSI721_EM_MECS_EVENT_GEN                         ((uint32_t)0x0001095c)
#define TSI721_EM_RST_PORT_STAT                          ((uint32_t)0x00010960)
#define TSI721_EM_RST_INT_EN                             ((uint32_t)0x00010968)
#define TSI721_EM_RST_PW_EN                              ((uint32_t)0x00010970)
#define TSI721_PW_BH                                     ((uint32_t)0x00010a00)
#define TSI721_PW_CTL                                    ((uint32_t)0x00010a04)
#define TSI721_PW_ROUTE                                  ((uint32_t)0x00010a08)
#define TSI721_PW_RX_STAT                                ((uint32_t)0x00010a10)
#define TSI721_PW_RX_EVENT_GEN                           ((uint32_t)0x00010a14)
#define TSI721_PW_RX_CAPT                                ((uint32_t)0x00010a20)
#define TSI721_LLM_BH                                    ((uint32_t)0x00010d00)
#define TSI721_MTC_WR_RESTRICT                           ((uint32_t)0x00010d10)
#define TSI721_MTC_PWR_RESTRICT                          ((uint32_t)0x00010d14)
#define TSI721_MTC_RD_RESTRICT                           ((uint32_t)0x00010d18)
#define TSI721_WHITEBOARD                                ((uint32_t)0x00010d24)
#define TSI721_PRESCALAR_SRV_CLK                         ((uint32_t)0x00010d30)
#define TSI721_REG_RST_CTL                               ((uint32_t)0x00010d34)
#define TSI721_LOCAL_ERR_DET                             ((uint32_t)0x00010d48)
#define TSI721_LOCAL_ERR_EN                              ((uint32_t)0x00010d4c)
#define TSI721_LOCAL_H_ADDR_CAPT                         ((uint32_t)0x00010d50)
#define TSI721_LOCAL_ADDR_CAPT                           ((uint32_t)0x00010d54)
#define TSI721_LOCAL_ID_CAPT                             ((uint32_t)0x00010d58)
#define TSI721_LOCAL_CTRL_CAPT                           ((uint32_t)0x00010d5c)
#define TSI721_FABRIC_BH                                 ((uint32_t)0x00010e00)
#define TSI721_PRBS_BH                                   ((uint32_t)0x00012000)
#define TSI721_PRBS_LANEX_CTRL(X)             ((uint32_t)(0x12004 + 0x010*(X)))
#define TSI721_PRBS_LANEX_SEED(X)             ((uint32_t)(0x12008 + 0x010*(X)))
#define TSI721_PRBS_LANEX_ERR_COUNT(X)        ((uint32_t)(0x1200c + 0x010*(X)))

/******************************************************/
/* TSI721 : S-RIO Register Bit Masks and Reset Values */
/*           definitions for every register           */
/******************************************************/

/* TSI721_DEV_ID : Register Bits Masks Definitions */
#define TSI721_DEV_IDENT_VEND                            ((uint32_t)0x0000ffff)
#define TSI721_DEV_IDENT_DEVI                            ((uint32_t)0xffff0000)
#define TSI721_DEVICE_VENDOR (0x00000038)
//#define TSI721_DEVICE_ID     (0x000080AB)

/* TSI721_DEV_INFO : Register Bits Masks Definitions */
#define TSI721_DEV_INFO_DEV_REV                          ((uint32_t)0xffffffff)

/* TSI721_ASBLY_ID : Register Bits Masks Definitions */
#define TSI721_ASBLY_ID_ASBLY_VEN_ID                     ((uint32_t)0x0000ffff)
#define TSI721_ASBLY_ID_ASBLY_ID                         ((uint32_t)0xffff0000)

/* TSI721_ASBLY_INFO : Register Bits Masks Definitions */
#define TSI721_ASSY_INF_EFB_PTR                          ((uint32_t)0x0000ffff)
#define TSI721_ASSY_INF_ASSY_REV                         ((uint32_t)0xffff0000)

/* TSI721_PE_FEAT : Register Bits Masks Definitions */
#define TSI721_PE_FEAT_EXT_AS                            ((uint32_t)0x00000007)
#define TSI721_PE_FEAT_EXT_FEA                           ((uint32_t)0x00000008)
#define TSI721_PE_FEAT_CTLS                              ((uint32_t)0x00000010)
#define TSI721_PE_FEAT_CRF                               ((uint32_t)0x00000020)
#define TSI721_PE_FEAT_FLOW_CTRL                         ((uint32_t)0x00000080)
#define TSI721_PE_FEAT_SRTC                              ((uint32_t)0x00000100)
#define TSI721_PE_FEAT_ERTC                              ((uint32_t)0x00000200)
#define TSI721_PE_FEAT_MC                                ((uint32_t)0x00000400)
#define TSI721_PE_FEAT_FLOW_ARB                          ((uint32_t)0x00000800)
#define TSI721_PE_FEAT_MULT_P                            ((uint32_t)0x08000000)
#define TSI721_PE_FEAT_SW                                ((uint32_t)0x10000000)
#define TSI721_PE_FEAT_PROC                              ((uint32_t)0x20000000)
#define TSI721_PE_FEAT_MEM                               ((uint32_t)0x40000000)
#define TSI721_PE_FEAT_BRDG                              ((uint32_t)0x80000000)

/* TSI721_SRC_OP : Register Bits Masks Definitions */
#define TSI721_SRC_OP_IMPLEMENT_DEF2                     ((uint32_t)0x00000003)
#define TSI721_SRC_OP_PORT_WR                            ((uint32_t)0x00000004)
#define TSI721_SRC_OP_A_SWAP                             ((uint32_t)0x00000008)
#define TSI721_SRC_OP_A_CLEAR                            ((uint32_t)0x00000010)
#define TSI721_SRC_OP_A_SET                              ((uint32_t)0x00000020)
#define TSI721_SRC_OP_A_DEC                              ((uint32_t)0x00000040)
#define TSI721_SRC_OP_A_INC                              ((uint32_t)0x00000080)
#define TSI721_SRC_OP_ATSWAP                             ((uint32_t)0x00000100)
#define TSI721_SRC_OP_ACSWAP                             ((uint32_t)0x00000200)
#define TSI721_SRC_OP_DBELL                              ((uint32_t)0x00000400)
#define TSI721_SRC_OP_D_MSG                              ((uint32_t)0x00000800)
#define TSI721_SRC_OP_WR_RES                             ((uint32_t)0x00001000)
#define TSI721_SRC_OP_STRM_WR                            ((uint32_t)0x00002000)
#define TSI721_SRC_OP_WRITE                              ((uint32_t)0x00004000)
#define TSI721_SRC_OP_READ                               ((uint32_t)0x00008000)
#define TSI721_SRC_OP_IMPLEMENT_DEF                      ((uint32_t)0x00030000)
#define TSI721_SRC_OP_DS                                 ((uint32_t)0x00040000)
#define TSI721_SRC_OP_DS_TM                              ((uint32_t)0x00080000)
#define TSI721_SRC_OP_RIO_RSVD_11                        ((uint32_t)0x00100000)
#define TSI721_SRC_OP_RIO_RSVD_10                        ((uint32_t)0x00200000)
#define TSI721_SRC_OP_G_TLB_SYNC                         ((uint32_t)0x00400000)
#define TSI721_SRC_OP_G_TLB_INVALIDATE                   ((uint32_t)0x00800000)
#define TSI721_SRC_OP_G_IC_INVALIDATE                    ((uint32_t)0x01000000)
#define TSI721_SRC_OP_G_IO_READ                          ((uint32_t)0x02000000)
#define TSI721_SRC_OP_G_DC_FLUSH                         ((uint32_t)0x04000000)
#define TSI721_SRC_OP_G_CASTOUT                          ((uint32_t)0x08000000)
#define TSI721_SRC_OP_G_DC_INVALIDATE                    ((uint32_t)0x10000000)
#define TSI721_SRC_OP_G_READ_OWN                         ((uint32_t)0x20000000)
#define TSI721_SRC_OP_G_IREAD                            ((uint32_t)0x40000000)
#define TSI721_SRC_OP_G_READ                             ((uint32_t)0x80000000)

/* TSI721_DEST_OP : Register Bits Masks Definitions */
#define TSI721_DEST_OP_IMPLEMENT_DEF2                    ((uint32_t)0x00000003)
#define TSI721_DEST_OP_PORT_WR                           ((uint32_t)0x00000004)
#define TSI721_DEST_OP_A_SWAP                            ((uint32_t)0x00000008)
#define TSI721_DEST_OP_A_CLEAR                           ((uint32_t)0x00000010)
#define TSI721_DEST_OP_A_SET                             ((uint32_t)0x00000020)
#define TSI721_DEST_OP_A_DEC                             ((uint32_t)0x00000040)
#define TSI721_DEST_OP_A_INC                             ((uint32_t)0x00000080)
#define TSI721_DEST_OP_ATSWAP                            ((uint32_t)0x00000100)
#define TSI721_DEST_OP_ACSWAP                            ((uint32_t)0x00000200)
#define TSI721_DEST_OP_DBELL                             ((uint32_t)0x00000400)
#define TSI721_DEST_OP_D_MSG                             ((uint32_t)0x00000800)
#define TSI721_DEST_OP_WR_RES                            ((uint32_t)0x00001000)
#define TSI721_DEST_OP_STRM_WR                           ((uint32_t)0x00002000)
#define TSI721_DEST_OP_WRITE                             ((uint32_t)0x00004000)
#define TSI721_DEST_OP_READ                              ((uint32_t)0x00008000)
#define TSI721_DEST_OP_IMPLEMENT_DEF                     ((uint32_t)0x00030000)
#define TSI721_DEST_OP_DS                                ((uint32_t)0x00040000)
#define TSI721_DEST_OP_DS_TM                             ((uint32_t)0x00080000)
#define TSI721_DEST_OP_RIO_RSVD_11                       ((uint32_t)0x00100000)
#define TSI721_DEST_OP_RIO_RSVD_10                       ((uint32_t)0x00200000)
#define TSI721_DEST_OP_G_TLB_SYNC                        ((uint32_t)0x00400000)
#define TSI721_DEST_OP_G_TLB_INVALIDATE                  ((uint32_t)0x00800000)
#define TSI721_DEST_OP_G_IC_INVALIDATE                   ((uint32_t)0x01000000)
#define TSI721_DEST_OP_G_IO_READ                         ((uint32_t)0x02000000)
#define TSI721_DEST_OP_G_DC_FLUSH                        ((uint32_t)0x04000000)
#define TSI721_DEST_OP_G_CASTOUT                         ((uint32_t)0x08000000)
#define TSI721_DEST_OP_G_DC_INVALIDATE                   ((uint32_t)0x10000000)
#define TSI721_DEST_OP_G_READ_OWN                        ((uint32_t)0x20000000)
#define TSI721_DEST_OP_G_IREAD                           ((uint32_t)0x40000000)
#define TSI721_DEST_OP_G_READ                            ((uint32_t)0x80000000)

/* TSI721_SR_XADDR : Register Bits Masks Definitions */
#define TSI721_SR_XADDR_EA_CTL                           ((uint32_t)0x00000007)

/* TSI721_BASE_ID : Register Bits Masks Definitions */
#define TSI721_BASE_ID_LAR_BASE_ID                       ((uint32_t)0x0000ffff)
#define TSI721_BASE_ID_BASE_ID                           ((uint32_t)0x00ff0000)

/* TSI721_HOST_BASE_ID_LOCK : Register Bits Masks Definitions */
#define TSI721_HOST_BASE_ID_LOCK_HOST_BASE_ID            ((uint32_t)0x0000ffff)

/* TSI721_COMP_TAG : Register Bits Masks Definitions */
#define TSI721_COMP_TAG_CTAG                             ((uint32_t)0xffffffff)

/* TSI721_SP_MB_HEAD : Register Bits Masks Definitions */
#define TSI721_SP_MB_HEAD_EF_ID                          ((uint32_t)0x0000ffff)
#define TSI721_SP_MB_HEAD_EF_PTR                         ((uint32_t)0xffff0000)

/* TSI721_SP_LT_CTL : Register Bits Masks Definitions */
#define TSI721_SP_LT_CTL_TVAL                            ((uint32_t)0xffffff00)

/* TSI721_SR_RSP_TO : Register Bits Masks Definitions */
#define TSI721_SR_RSP_TO_RSP_TO                          ((uint32_t)0x00ffffff)

#define TSI721_SR_RSP_TO_TICK_NSEC 188
#define TSI721_NSEC_TO_RTO(x) \
	(((x + TSI721_SR_RSP_TO_TICK_NSEC - 1) / TSI721_SR_RSP_TO_TICK_NSEC) \
	& TSI721_SR_RSP_TO_RSP_TO)

/* TSI721_SP_GEN_CTL : Register Bits Masks Definitions */
#define TSI721_SP_GEN_CTL_DISC                           ((uint32_t)0x20000000)
#define TSI721_SP_GEN_CTL_MAST_EN                        ((uint32_t)0x40000000)
#define TSI721_SP_GEN_CTL_HOST                           ((uint32_t)0x80000000)

/* TSI721_SP_LM_REQ : Register Bits Masks Definitions */
#define TSI721_SP_LM_REQ_CMD                             ((uint32_t)0x00000007)

/* TSI721_SP_LM_RESP : Register Bits Masks Definitions */
#define TSI721_SP_LM_RESP_LINK_STAT                      ((uint32_t)0x0000001f)
#define TSI721_SP_LM_RESP_ACK_ID_STAT                    ((uint32_t)0x000007e0)
#define TSI721_SP_LM_RESP_RESP_VLD                       ((uint32_t)0x80000000)

/* TSI721_SP_ACKID_STAT : Register Bits Masks Definitions */
#define TSI721_SP_ACKID_STAT_OUTB_ACKID                  ((uint32_t)0x0000003f)
#define TSI721_SP_ACKID_STAT_OUTSTD_ACKID                ((uint32_t)0x00003f00)
#define TSI721_SP_ACKID_STAT_INB_ACKID                   ((uint32_t)0x3f000000)
#define TSI721_SP_ACKID_STAT_CLR_OUTSTD_ACKID            ((uint32_t)0x80000000)

/* TSI721_SP_CTL2 : Register Bits Masks Definitions */
#define TSI721_SP_CTL2_RTEC_EN                           ((uint32_t)0x00000001)
#define TSI721_SP_CTL2_RTEC                              ((uint32_t)0x00000002)
#define TSI721_SP_CTL2_D_SCRM_DIS                        ((uint32_t)0x00000004)
#define TSI721_SP_CTL2_INACT_EN                          ((uint32_t)0x00000008)
#define TSI721_SP_CTL2_GB_6P25_EN                        ((uint32_t)0x00010000)
#define TSI721_SP_CTL2_GB_6P25                           ((uint32_t)0x00020000)
#define TSI721_SP_CTL2_GB_5P0_EN                         ((uint32_t)0x00040000)
#define TSI721_SP_CTL2_GB_5P0                            ((uint32_t)0x00080000)
#define TSI721_SP_CTL2_GB_3P125_EN                       ((uint32_t)0x00100000)
#define TSI721_SP_CTL2_GB_3P125                          ((uint32_t)0x00200000)
#define TSI721_SP_CTL2_GB_2P5_EN                         ((uint32_t)0x00400000)
#define TSI721_SP_CTL2_GB_2P5                            ((uint32_t)0x00800000)
#define TSI721_SP_CTL2_GB_1P25_EN                        ((uint32_t)0x01000000)
#define TSI721_SP_CTL2_GB_1P25                           ((uint32_t)0x02000000)
#define TSI721_SP_CTL2_BAUD_DISC                         ((uint32_t)0x08000000)
#define TSI721_SP_CTL2_BAUD_SEL                          ((uint32_t)0xf0000000)

/* TSI721_SP_ERR_STAT : Register Bits Masks Definitions */
#define TSI721_SP_ERR_STAT_PORT_UNIT                     ((uint32_t)0x00000001)
#define TSI721_SP_ERR_STAT_PORT_OK                       ((uint32_t)0x00000002)
#define TSI721_SP_ERR_STAT_PORT_ERR                      ((uint32_t)0x00000004)
#define TSI721_SP_ERR_STAT_PORT_UNAVL                    ((uint32_t)0x00000008)
#define TSI721_SP_ERR_STAT_PORT_W_P                      ((uint32_t)0x00000010)
#define TSI721_SP_ERR_STAT_INPUT_ERR_STOP                ((uint32_t)0x00000100)
#define TSI721_SP_ERR_STAT_INPUT_ERR_ENCTR               ((uint32_t)0x00000200)
#define TSI721_SP_ERR_STAT_INPUT_RS                      ((uint32_t)0x00000400)
#define TSI721_SP_ERR_STAT_OUTPUT_ERR_STOP               ((uint32_t)0x00010000)
#define TSI721_SP_ERR_STAT_OUTPUT_ERR_ENCTR              ((uint32_t)0x00020000)
#define TSI721_SP_ERR_STAT_OUTPUT_RS                     ((uint32_t)0x00040000)
#define TSI721_SP_ERR_STAT_OUTPUT_R                      ((uint32_t)0x00080000)
#define TSI721_SP_ERR_STAT_OUTPUT_RE                     ((uint32_t)0x00100000)
#define TSI721_SP_ERR_STAT_OUTPUT_DEGR                   ((uint32_t)0x01000000)
#define TSI721_SP_ERR_STAT_OUTPUT_FAIL                   ((uint32_t)0x02000000)
#define TSI721_SP_ERR_STAT_OUTPUT_DROP                   ((uint32_t)0x04000000)
#define TSI721_SP_ERR_STAT_TXFC                          ((uint32_t)0x08000000)
#define TSI721_SP_ERR_STAT_IDLE_SEQ                      ((uint32_t)0x20000000)
#define TSI721_SP_ERR_STAT_IDLE2_EN                      ((uint32_t)0x40000000)
#define TSI721_SP_ERR_STAT_IDLE2                         ((uint32_t)0x80000000)

/* TSI721_SP_CTL : Register Bits Masks Definitions */
#define TSI721_SP_CTL_PTYP                               ((uint32_t)0x00000001)
#define TSI721_SP_CTL_PORT_LOCKOUT                       ((uint32_t)0x00000002)
#define TSI721_SP_CTL_DROP_EN                            ((uint32_t)0x00000004)
#define TSI721_SP_CTL_STOP_FAIL_EN                       ((uint32_t)0x00000008)
#define TSI721_SP_CTL_PORT_WIDTH2                        ((uint32_t)0x00003000)
#define TSI721_SP_CTL_OVER_PWIDTH2                       ((uint32_t)0x0000c000)
#define TSI721_SP_CTL_FLOW_ARB                           ((uint32_t)0x00010000)
#define TSI721_SP_CTL_ENUM_B                             ((uint32_t)0x00020000)
#define TSI721_SP_CTL_FLOW_CTRL                          ((uint32_t)0x00040000)
#define TSI721_SP_CTL_MULT_CS                            ((uint32_t)0x00080000)
#define TSI721_SP_CTL_ERR_DIS                            ((uint32_t)0x00100000)
#define TSI721_SP_CTL_INP_EN                             ((uint32_t)0x00200000)
#define TSI721_SP_CTL_OTP_EN                             ((uint32_t)0x00400000)
#define TSI721_SP_CTL_PORT_DIS                           ((uint32_t)0x00800000)
#define TSI721_SP_CTL_OVER_PWIDTH                        ((uint32_t)0x07000000)
#define TSI721_SP_CTL_INIT_PWIDTH                        ((uint32_t)0x38000000)
#define TSI721_SP_CTL_PORT_WIDTH                         ((uint32_t)0xc0000000)

/* TSI721_ERR_RPT_BH : Register Bits Masks Definitions */
#define TSI721_ERR_RPT_BH_EF_ID                          ((uint32_t)0x0000ffff)
#define TSI721_ERR_RPT_BH_EF_PTR                         ((uint32_t)0xffff0000)

/* TSI721_ERR_DET : Register Bits Masks Definitions */
#define TSI721_ERR_DET_IMP                               ((uint32_t)0x00000001)

/* TSI721_ERR_EN : Register Bits Masks Definitions */
#define TSI721_ERR_EN_IMP_EN                             ((uint32_t)0x00000001)

/* TSI721_H_ADDR_CAPT : Register Bits Masks Definitions */
#define TSI721_H_ADDR_CAPT_ADDR                          ((uint32_t)0xffffffff)

/* TSI721_ADDR_CAPT : Register Bits Masks Definitions */
#define TSI721_ADDR_CAPT_XAMSBS                          ((uint32_t)0x00000003)
#define TSI721_ADDR_CAPT_ADDR                            ((uint32_t)0xfffffff8)

/* TSI721_ID_CAPT : Register Bits Masks Definitions */
#define TSI721_ID_CAPT_SRC_ID                            ((uint32_t)0x000000ff)
#define TSI721_ID_CAPT_MSB_SRC_ID                        ((uint32_t)0x0000ff00)
#define TSI721_ID_CAPT_DEST_ID                           ((uint32_t)0x00ff0000)
#define TSI721_ID_CAPT_MSB_DEST_ID                       ((uint32_t)0xff000000)

/* TSI721_CTRL_CAPT : Register Bits Masks Definitions */
#define TSI721_CTRL_CAPT_MESSAGE_INFO                    ((uint32_t)0x00ff0000)
#define TSI721_CTRL_CAPT_TTYPE                           ((uint32_t)0x0f000000)
#define TSI721_CTRL_CAPT_FTYPE                           ((uint32_t)0xf0000000)

/* TSI721_PW_TGT_ID : Register Bits Masks Definitions */
#define TSI721_PW_TGT_ID_LRG_TRANS                       ((uint32_t)0x00008000)
#define TSI721_PW_TGT_ID_PW_TGT_ID                       ((uint32_t)0x00ff0000)
#define TSI721_PW_TGT_ID_MSB_PW_ID                       ((uint32_t)0xff000000)
#define TSI721_PW_TGT_ID_DEV16_ID                        ((uint32_t)0xffff0000)

/* TSI721_SP_ERR_DET : Register Bits Masks Definitions */
#define TSI721_SP_ERR_DET_LINK_TO                        ((uint32_t)0x00000001)
#define TSI721_SP_ERR_DET_CS_ACK_ILL                     ((uint32_t)0x00000002)
#define TSI721_SP_ERR_DET_DELIN_ERR                      ((uint32_t)0x00000004)
#define TSI721_SP_ERR_DET_PROT_ERR                       ((uint32_t)0x00000010)
#define TSI721_SP_ERR_DET_LR_ACKID_ILL                   ((uint32_t)0x00000020)
#define TSI721_SP_ERR_DET_DSCRAM_LOS                     ((uint32_t)0x00004000)
#define TSI721_SP_ERR_DET_PKT_ILL_SIZE                   ((uint32_t)0x00020000)
#define TSI721_SP_ERR_DET_PKT_CRC_ERR                    ((uint32_t)0x00040000)
#define TSI721_SP_ERR_DET_PKT_ILL_ACKID                  ((uint32_t)0x00080000)
#define TSI721_SP_ERR_DET_CS_NOT_ACC                     ((uint32_t)0x00100000)
#define TSI721_SP_ERR_DET_CS_ILL_ID                      ((uint32_t)0x00200000)
#define TSI721_SP_ERR_DET_CS_CRC_ERR                     ((uint32_t)0x00400000)
#define TSI721_SP_ERR_DET_IMP_SPEC                       ((uint32_t)0x80000000)

/* TSI721_SP_RATE_EN : Register Bits Masks Definitions */
#define TSI721_SP_RATE_EN_LINK_TO_EN                     ((uint32_t)0x00000001)
#define TSI721_SP_RATE_EN_CS_ACK_ILL_EN                  ((uint32_t)0x00000002)
#define TSI721_SP_RATE_EN_DELIN_ERR_E                    ((uint32_t)0x00000004)
#define TSI721_SP_RATE_EN_PROT_ERR_EN                    ((uint32_t)0x00000010)
#define TSI721_SP_RATE_EN_LR_ACKID_ILL_EN                ((uint32_t)0x00000020)
#define TSI721_SP_RATE_EN_DSCRAM_LOS_EN                  ((uint32_t)0x00004000)
#define TSI721_SP_RATE_EN_PKT_ILL_SIZE_EN                ((uint32_t)0x00020000)
#define TSI721_SP_RATE_EN_PKT_CRC_ERR_EN                 ((uint32_t)0x00040000)
#define TSI721_SP_RATE_EN_PKT_ILL_ACKID_EN               ((uint32_t)0x00080000)
#define TSI721_SP_RATE_EN_CS_NOT_ACC_EN                  ((uint32_t)0x00100000)
#define TSI721_SP_RATE_EN_CS_ILL_ID_EN                   ((uint32_t)0x00200000)
#define TSI721_SP_RATE_EN_CS_CRC_ERR_EN                  ((uint32_t)0x00400000)
#define TSI721_SP_RATE_EN_IMP_SPEC_EN                    ((uint32_t)0x80000000)

/* TSI721_SP_ERR_ATTR_CAPT : Register Bits Masks Definitions */
#define TSI721_SP_ERR_ATTR_CAPT_VAL_CAPT                 ((uint32_t)0x00000001)
#define TSI721_SP_ERR_ATTR_CAPT_IMPL_DEP                 ((uint32_t)0x00fffff0)
#define TSI721_SP_ERR_ATTR_CAPT_ERR_TYPE                 ((uint32_t)0x1f000000)
#define TSI721_SP_ERR_ATTR_CAPT_INFO_TYPE                ((uint32_t)0xe0000000)

/* TSI721_SP_ERR_CAPT_0 : Register Bits Masks Definitions */
#define TSI721_SP_ERR_CAPT_0_CAPT_0                      ((uint32_t)0xffffffff)

/* TSI721_SP_ERR_CAPT_1 : Register Bits Masks Definitions */
#define TSI721_SP_ERR_CAPT_1_CAPT_1                      ((uint32_t)0xffffffff)

/* TSI721_SP_ERR_CAPT_2 : Register Bits Masks Definitions */
#define TSI721_SP_ERR_CAPT_2_CAPT_2                      ((uint32_t)0xffffffff)

/* TSI721_SP_ERR_CAPT_3 : Register Bits Masks Definitions */
#define TSI721_SP_ERR_CAPT_3_CAPT_3                      ((uint32_t)0xffffffff)

/* TSI721_SP_ERR_RATE : Register Bits Masks Definitions */
#define TSI721_SP_ERR_RATE_ERR_RATE_CNT                  ((uint32_t)0x000000ff)
#define TSI721_SP_ERR_RATE_PEAK                          ((uint32_t)0x0000ff00)
#define TSI721_SP_ERR_RATE_ERR_RR                        ((uint32_t)0x00030000)
#define TSI721_SP_ERR_RATE_ERR_RB                        ((uint32_t)0xff000000)

/* TSI721_SP_ERR_THRESH : Register Bits Masks Definitions */
#define TSI721_SP_ERR_THRESH_ERR_RDT                     ((uint32_t)0x00ff0000)
#define TSI721_SP_ERR_THRESH_ERR_RFT                     ((uint32_t)0xff000000)

/* TSI721 Port write field definitions */
#define TSI721_PW_TLM_PW                               TSI721_PLM_STATUS_TLM_PW
#define TSI721_PW_PBM_PW                               TSI721_PLM_STATUS_PBM_PW
#define TSI721_PW_RST_REQ                             TSI721_PLM_STATUS_RST_REQ
#define TSI721_PW_OUTPUT_DEGR                     TSI721_PLM_STATUS_OUTPUT_DEGR
#define TSI721_PW_OUTPUT_FAIL                     TSI721_PLM_STATUS_OUTPUT_FAIL
#define TSI721_PW_PORT_ERR                           TSI721_PLM_STATUS_PORT_ERR
#define TSI721_PW_DLT                                     TSI721_PLM_STATUS_DLT
#define TSI721_PW_LINK_INIT                         TSI721_PLM_STATUS_LINK_INIT
#define TSI721_PW_MAX_DENIAL                       TSI721_PLM_STATUS_MAX_DENIAL
#define TSI721_PW_RCS                                    ((uint32_t)0x00000400)
#define TSI721_PW_LOCALOG                                ((uint32_t)0x00000100)

/* TSI721_PER_LANE_BH : Register Bits Masks Definitions */
#define TSI721_PER_LANE_BH_EF_ID                         ((uint32_t)0x0000ffff)
#define TSI721_PER_LANE_BH_EF_PTR                        ((uint32_t)0xffff0000)

/* TSI721_LANEX_STAT0 : Register Bits Masks Definitions */
#define TSI721_LANEX_STAT0_STAT2_7                       ((uint32_t)0x00000007)
#define TSI721_LANEX_STAT0_STAT1                         ((uint32_t)0x00000008)
#define TSI721_LANEX_STAT0_CHG_TRN                       ((uint32_t)0x00000040)
#define TSI721_LANEX_STAT0_CHG_SYNC                      ((uint32_t)0x00000080)
#define TSI721_LANEX_STAT0_ERR_CNT                       ((uint32_t)0x00000f00)
#define TSI721_LANEX_STAT0_RX_RDY                        ((uint32_t)0x00001000)
#define TSI721_LANEX_STAT0_RX_SYNC                       ((uint32_t)0x00002000)
#define TSI721_LANEX_STAT0_RX_TRN                        ((uint32_t)0x00004000)
#define TSI721_LANEX_STAT0_RX_INV                        ((uint32_t)0x00008000)
#define TSI721_LANEX_STAT0_RX_TYPE                       ((uint32_t)0x00030000)
#define TSI721_LANEX_STAT0_TX_MODE                       ((uint32_t)0x00040000)
#define TSI721_LANEX_STAT0_TX_TYPE                       ((uint32_t)0x00080000)
#define TSI721_LANEX_STAT0_LANE_NUM                      ((uint32_t)0x00f00000)
#define TSI721_LANEX_STAT0_PORT_NUM                      ((uint32_t)0xff000000)


/* TSI721_LANEX_STAT1 : Register Bits Masks Definitions */
#define TSI721_LANEX_STAT1_LP_SCRM                       ((uint32_t)0x00008000)
#define TSI721_LANEX_STAT1_LP_TAP_P1                     ((uint32_t)0x00030000)
#define TSI721_LANEX_STAT1_LP_TAP_M1                     ((uint32_t)0x000c0000)
#define TSI721_LANEX_STAT1_LP_LANE_NUM                   ((uint32_t)0x00f00000)
#define TSI721_LANEX_STAT1_LP_WIDTH                      ((uint32_t)0x07000000)
#define TSI721_LANEX_STAT1_LP_RX_TRN                     ((uint32_t)0x08000000)
#define TSI721_LANEX_STAT1_IMPL_SPEC                     ((uint32_t)0x10000000)
#define TSI721_LANEX_STAT1_CHG                           ((uint32_t)0x20000000)
#define TSI721_LANEX_STAT1_INFO_OK                       ((uint32_t)0x40000000)
#define TSI721_LANEX_STAT1_IDLE2                         ((uint32_t)0x80000000)

/* TSI721_PLM_BH : Register Bits Masks Definitions */
#define TSI721_PLM_BH_BLK_TYPE                           ((uint32_t)0x00000fff)
#define TSI721_PLM_BH_BLK_REV                            ((uint32_t)0x0000f000)
#define TSI721_PLM_BH_NEXT_BLK_PTR                       ((uint32_t)0xffff0000)

/* TSI721_PLM_IMP_SPEC_CTL : Register Bits Masks Definitions */
#define TSI721_PLM_IMP_SPEC_CTL_DLT_THRESH               ((uint32_t)0x0000ffff)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_RX                  ((uint32_t)0x00030000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_TX                  ((uint32_t)0x000c0000)
#define TSI721_PLM_IMP_SPEC_CTL_SELF_RST                 ((uint32_t)0x00100000)
#define TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST            ((uint32_t)0x00200000)
#define TSI721_PLM_IMP_SPEC_CTL_RESET_REG                ((uint32_t)0x00400000)
#define TSI721_PLM_IMP_SPEC_CTL_LLB_EN                   ((uint32_t)0x00800000)
#define TSI721_PLM_IMP_SPEC_CTL_TX_BYPASS                ((uint32_t)0x01000000)
#define TSI721_PLM_IMP_SPEC_CTL_FORCE_REINIT             ((uint32_t)0x04000000)
#define TSI721_PLM_IMP_SPEC_CTL_SOFT_RST                 ((uint32_t)0x02000000)
#define TSI721_PLM_IMP_SPEC_CTL_DLB_EN                   ((uint32_t)0x10000000)
#define TSI721_PLM_IMP_SPEC_CTL_USE_IDLE1                ((uint32_t)0x20000000)
#define TSI721_PLM_IMP_SPEC_CTL_USE_IDLE2                ((uint32_t)0x40000000)
#define TSI721_PLM_IMP_SPEC_CTL_PAYL_CAP                 ((uint32_t)0x80000000)

// Number of microseconds in each DLT tick.
// May be used to compute default DLT timeouts.
#define TSI721_PLM_IMP_SPEC_CTL_DLT_TICK_USEC ((uint32_t)(256))
#define TSI721_PLM_DLT_TICK_NSEC (TSI721_PLM_IMP_SPEC_CTL_DLT_TICK_USEC * 1000)

#define TSI721_NSEC_TO_DLT(x) \
		((uint32_t)(((x + TSI721_PLM_DLT_TICK_NSEC - 1)		\
				/ TSI721_PLM_DLT_TICK_NSEC)		\
				& TSI721_PLM_IMP_SPEC_CTL_DLT_THRESH))

#define TSI721_DLT_TO_NSEC(x) \
		((uint32_t)((x & TSI721_PLM_IMP_SPEC_CTL_DLT_THRESH)	\
				* TSI721_PLM_DLT_TICK_NSEC))

#define TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_NONE             ((uint32_t)0x00000000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_1032             ((uint32_t)0x00010000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_3210             ((uint32_t)0x00020000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_2301             ((uint32_t)0x00030000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_TX_NONE             ((uint32_t)0x00000000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_TX_1032             ((uint32_t)0x00040000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_TX_3210             ((uint32_t)0x00080000)
#define TSI721_PLM_IMP_SPEC_CTL_SWAP_TX_2301             ((uint32_t)0x000C0000)

/* TSI721_PLM_STATUS : Register Bits Masks Definitions */
#define TSI721_PLM_STATUS_TLM_INT                        ((uint32_t)0x00000400)
#define TSI721_PLM_STATUS_PBM_INT                        ((uint32_t)0x00000800)
#define TSI721_PLM_STATUS_MECS                           ((uint32_t)0x00001000)
#define TSI721_PLM_STATUS_TLM_PW                         ((uint32_t)0x00004000)
#define TSI721_PLM_STATUS_PBM_PW                         ((uint32_t)0x00008000)
#define TSI721_PLM_STATUS_RST_REQ                        ((uint32_t)0x00010000)
#define TSI721_PLM_STATUS_OUTPUT_DEGR                    ((uint32_t)0x01000000)
#define TSI721_PLM_STATUS_OUTPUT_FAIL                    ((uint32_t)0x02000000)
#define TSI721_PLM_STATUS_PORT_ERR                       ((uint32_t)0x04000000)
#define TSI721_PLM_STATUS_DLT                            ((uint32_t)0x08000000)
#define TSI721_PLM_STATUS_LINK_INIT                      ((uint32_t)0x10000000)
#define TSI721_PLM_STATUS_MAX_DENIAL                     ((uint32_t)0x80000000)

/* TSI721_PLM_INT_ENABLE : Register Bits Masks Definitions */
#define TSI721_PLM_INT_ENABLE_OUTPUT_DEGR                ((uint32_t)0x01000000)
#define TSI721_PLM_INT_ENABLE_OUTPUT_FAIL                ((uint32_t)0x02000000)
#define TSI721_PLM_INT_ENABLE_PORT_ERR                   ((uint32_t)0x04000000)
#define TSI721_PLM_INT_ENABLE_DLT                        ((uint32_t)0x08000000)
#define TSI721_PLM_INT_ENABLE_LINK_INIT                  ((uint32_t)0x10000000)
#define TSI721_PLM_INT_ENABLE_MAX_DENIAL                 ((uint32_t)0x80000000)

/* TSI721_PLM_PW_ENABLE : Register Bits Masks Definitions */
#define TSI721_PLM_PW_ENABLE_OUTPUT_DEGR                 ((uint32_t)0x01000000)
#define TSI721_PLM_PW_ENABLE_OUTPUT_FAIL                 ((uint32_t)0x02000000)
#define TSI721_PLM_PW_ENABLE_PORT_ERR                    ((uint32_t)0x04000000)
#define TSI721_PLM_PW_ENABLE_DLT                         ((uint32_t)0x08000000)
#define TSI721_PLM_PW_ENABLE_LINK_INIT                   ((uint32_t)0x10000000)
#define TSI721_PLM_PW_ENABLE_MAX_DENIAL                  ((uint32_t)0x80000000)

/* TSI721_PLM_EVENT_GEN : Register Bits Masks Definitions */
#define TSI721_PLM_EVENT_GEN_RST_REQ                     ((uint32_t)0x00010000)
#define TSI721_PLM_EVENT_GEN_OUTPUT_DEGR                 ((uint32_t)0x01000000)
#define TSI721_PLM_EVENT_GEN_OUTPUT_FAIL                 ((uint32_t)0x02000000)
#define TSI721_PLM_EVENT_GEN_PORT_ERR                    ((uint32_t)0x04000000)
#define TSI721_PLM_EVENT_GEN_DLT                         ((uint32_t)0x08000000)
#define TSI721_PLM_EVENT_GEN_LINK_INIT                   ((uint32_t)0x10000000)
#define TSI721_PLM_EVENT_GEN_MAX_DENIAL                  ((uint32_t)0x80000000)

/* TSI721_PLM_ALL_INT_EN : Register Bits Masks Definitions */
#define TSI721_PLM_ALL_INT_EN_IRQ_EN                     ((uint32_t)0x00000001)

/* TSI721_PLM_ALL_PW_EN : Register Bits Masks Definitions */
#define TSI721_PLM_ALL_PW_EN_PW_EN                       ((uint32_t)0x00000001)

/* TSI721_PLM_DISCOVERY_TIMER : Register Bits Masks Definitions */
#define TSI721_PLM_DISCOVERY_TIMER_DISCOVERY_TIMER       ((uint32_t)0xf0000000)

/* TSI721_PLM_SILENCE_TIMER : Register Bits Masks Definitions */
#define TSI721_PLM_SILENCE_TIMER_SILENCE_TIMER           ((uint32_t)0xf0000000)

/* TSI721_PLM_VMIN_EXP : Register Bits Masks Definitions */
#define TSI721_PLM_VMIN_EXP_MMAX                         ((uint32_t)0x00000f00)
#define TSI721_PLM_VMIN_EXP_IMAX                         ((uint32_t)0x000f0000)
#define TSI721_PLM_VMIN_EXP_VMIN_EXP                     ((uint32_t)0x1f000000)

/* TSI721_PLM_POL_CTL : Register Bits Masks Definitions */
#define TSI721_PLM_POL_CTL_RX0_POL                       ((uint32_t)0x00000001)
#define TSI721_PLM_POL_CTL_RX1_POL                       ((uint32_t)0x00000002)
#define TSI721_PLM_POL_CTL_RX2_POL                       ((uint32_t)0x00000004)
#define TSI721_PLM_POL_CTL_RX3_POL                       ((uint32_t)0x00000008)
#define TSI721_PLM_POL_CTL_TX0_POL                       ((uint32_t)0x00010000)
#define TSI721_PLM_POL_CTL_TX1_POL                       ((uint32_t)0x00020000)
#define TSI721_PLM_POL_CTL_TX2_POL                       ((uint32_t)0x00040000)
#define TSI721_PLM_POL_CTL_TX3_POL                       ((uint32_t)0x00080000)

/* TSI721_PLM_DENIAL_CTL : Register Bits Masks Definitions */
#define TSI721_PLM_DENIAL_CTL_DENIAL_THRESH              ((uint32_t)0x000000ff)
#define TSI721_PLM_DENIAL_CTL_CNT_RTY                    ((uint32_t)0x10000000)
#define TSI721_PLM_DENIAL_CTL_CNT_PNA                    ((uint32_t)0x20000000)

/* TSI721_PLM_RCVD_MECS : Register Bits Masks Definitions */
#define TSI721_PLM_RCVD_MECS_CMD_STAT                    ((uint32_t)0x000000ff)

/* TSI721_PLM_MECS_FWD : Register Bits Masks Definitions */
#define TSI721_PLM_MECS_FWD_MULT_CS                      ((uint32_t)0x00000001)
#define TSI721_PLM_MECS_FWD_SUBSCRIPTION                 ((uint32_t)0x000000fe)

/* TSI721_PLM_LONG_CS_TX1 : Register Bits Masks Definitions */
#define TSI721_PLM_LONG_CS_TX1_CMD                       ((uint32_t)0x00000007)
#define TSI721_PLM_LONG_CS_TX1_STYPE_1                   ((uint32_t)0x00000070)
#define TSI721_PLM_LONG_CS_TX1_CS_EMB                    ((uint32_t)0x00000100)
#define TSI721_PLM_LONG_CS_TX1_PAR_1                     ((uint32_t)0x0003f000)
#define TSI721_PLM_LONG_CS_TX1_PAR_0                     ((uint32_t)0x03f00000)
#define TSI721_PLM_LONG_CS_TX1_STYPE_0                   ((uint32_t)0x70000000)

#define TSI721_MAGIC_CS \
((uint32_t)( \
 (((uint32_t)(stype0_pna     ) << 28) & TSI721_PLM_LONG_CS_TX1_STYPE_0) | \
 (((uint32_t)(0x00000000     ) << 20) & TSI721_PLM_LONG_CS_TX1_PAR_0  ) | \
 (((uint32_t)(PNA_GENERAL_ERR) << 12) & TSI721_PLM_LONG_CS_TX1_PAR_1  ) | \
 (((uint32_t)(stype1_lreq    ) <<  4) & TSI721_PLM_LONG_CS_TX1_STYPE_1) | \
 (((uint32_t)(STYPE1_LREQ_CMD_PORT_STAT)) & TSI721_PLM_LONG_CS_TX1_CMD) \
))

#define TSI721_MECS_CS \
((uint32_t)( \
 (((uint32_t)(stype0_status) << 28) & TSI721_PLM_LONG_CS_TX1_STYPE_0) | \
 (((uint32_t)(0x00         ) << 20) & TSI721_PLM_LONG_CS_TX1_PAR_0  ) | \
 (((uint32_t)(0x1F         ) << 12) & TSI721_PLM_LONG_CS_TX1_PAR_1  ) | \
 (((uint32_t)(stype1_mecs  ) <<  4) & TSI721_PLM_LONG_CS_TX1_STYPE_1) | \
 (((uint32_t)(0x0          )) & TSI721_PLM_LONG_CS_TX1_CMD) \
))

/* TSI721_PLM_LONG_CS_TX2 : Register Bits Masks Definitions */
#define TSI721_PLM_LONG_CS_TX2_PARM                      ((uint32_t)0x07ff0000)
#define TSI721_PLM_LONG_CS_TX2_STYPE2                    ((uint32_t)0x70000000)

/* TSI721_TLM_BH : Register Bits Masks Definitions */
#define TSI721_TLM_BH_BLK_TYPE                           ((uint32_t)0x00000fff)
#define TSI721_TLM_BH_BLK_REV                            ((uint32_t)0x0000f000)
#define TSI721_TLM_BH_NEXT_BLK_PTR                       ((uint32_t)0xffff0000)

/* TSI721_TLM_SP_CONTROL : Register Bits Masks Definitions */
#define TSI721_TLM_SP_CONTROL_LENGTH                     ((uint32_t)0x0000f000)
#define TSI721_TLM_SP_CONTROL_MTC_TGT_ID_DIS             ((uint32_t)0x00100000)
#define TSI721_TLM_SP_CONTROL_TGT_ID_DIS                 ((uint32_t)0x00200000)
#define TSI721_TLM_SP_CONTROL_PORTGROUP_SELECT           ((uint32_t)0x40000000)

/* TSI721_TLM_SP_STATUS : Register Bits Masks Definitions */
#define TSI721_TLM_SP_STATUS_IG_BRR_FILTER               ((uint32_t)0x00100000)
#define TSI721_TLM_SP_STATUS_IG_BAD_VC                   ((uint32_t)0x80000000)

/* TSI721_TLM_SP_INT_ENABLE : Register Bits Masks Definitions */
#define TSI721_TLM_SP_INT_ENABLE_IG_BRR_FILTER           ((uint32_t)0x00100000)
#define TSI721_TLM_SP_INT_ENABLE_IG_BAD_VC               ((uint32_t)0x80000000)

/* TSI721_TLM_SP_PW_ENABLE : Register Bits Masks Definitions */
#define TSI721_TLM_SP_PW_ENABLE_IG_BRR_FILTER            ((uint32_t)0x00100000)
#define TSI721_TLM_SP_PW_ENABLE_IG_BAD_VC                ((uint32_t)0x80000000)

/* TSI721_TLM_SP_EVENT_GEN : Register Bits Masks Definitions */
#define TSI721_TLM_SP_EVENT_GEN_IG_BRR_FILTER            ((uint32_t)0x00100000)
#define TSI721_TLM_SP_EVENT_GEN_IG_BAD_VC                ((uint32_t)0x80000000)

/* TSI721_TLM_SP_BRR_CTLX : Register Bits Masks Definitions */
#define TSI721_TLM_SP_BRR_CTLX_PRIVATE                   ((uint32_t)0x01000000)
#define TSI721_TLM_SP_BRR_CTLX_ROUTE_MR_TO_LLM           ((uint32_t)0x04000000)
#define TSI721_TLM_SP_BRR_CTLX_ENABLE                    ((uint32_t)0x80000000)

/* TSI721_TLM_SP_BRR_PATTERN_MATCHX : Register Bits Masks Definitions */
#define TSI721_TLM_SP_BRR_PATTERN_MATCHX_MATCH           ((uint32_t)0x0000ffff)
#define TSI721_TLM_SP_BRR_PATTERN_MATCHX_PATTERN         ((uint32_t)0xffff0000)

/* TSI721_TLM_SP_FTYPE_FILTER_CTL : Register Bits Masks Definitions */
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F15_IMPLEMENTATION ((uint32_t)0x00000002)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F14_RSVD          ((uint32_t)0x00000004)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F13_OTHER         ((uint32_t)0x00000008)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F13_RESPONSE_DATA ((uint32_t)0x00000010)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F13_RESPONSE      ((uint32_t)0x00000020)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F12_RSVD          ((uint32_t)0x00000040)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F11_MESSAGE       ((uint32_t)0x00000080)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F10_DOORBELL      ((uint32_t)0x00000100)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F9_DATA_STREAMING ((uint32_t)0x00000200)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_OTHER          ((uint32_t)0x00000400)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_PWR            ((uint32_t)0x00000800)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MWR            ((uint32_t)0x00001000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MRR            ((uint32_t)0x00002000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MW             ((uint32_t)0x00004000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MR             ((uint32_t)0x00008000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F7_FLOW           ((uint32_t)0x00010000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F6_STREAMING_WRITE ((uint32_t)0x00020000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F5_OTHER          ((uint32_t)0x00040000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F5_ATOMIC         ((uint32_t)0x00080000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F5_NWRITE_R       ((uint32_t)0x00100000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F5_NWRITE         ((uint32_t)0x00200000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F5_GSM            ((uint32_t)0x00400000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F4_RSVD           ((uint32_t)0x00800000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F3_RSVD           ((uint32_t)0x01000000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F2_ATOMIC         ((uint32_t)0x02000000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F2_NREAD          ((uint32_t)0x04000000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F2_GSM            ((uint32_t)0x08000000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F1_ALL            ((uint32_t)0x10000000)
#define TSI721_TLM_SP_FTYPE_FILTER_CTL_F0_IMPLEMENTATION ((uint32_t)0x40000000)

/* TSI721_PBM_BH : Register Bits Masks Definitions */
#define TSI721_PBM_BH_BLK_TYPE                           ((uint32_t)0x00000fff)
#define TSI721_PBM_BH_BLK_REV                            ((uint32_t)0x0000f000)
#define TSI721_PBM_BH_NEXT_BLK_PTR                       ((uint32_t)0xffff0000)

/* TSI721_PBM_SP_CONTROL : Register Bits Masks Definitions */
#define TSI721_PBM_SP_CONTROL_EG_REORDER_STICK           ((uint32_t)0x00000007)
#define TSI721_PBM_SP_CONTROL_EG_REORDER_MODE            ((uint32_t)0x00000030)
#define TSI721_PBM_SP_CONTROL_IG_BACKPRESSURE_ON_FATAL   ((uint32_t)0x00010000)

/* TSI721_PBM_SP_STATUS : Register Bits Masks Definitions */
#define TSI721_PBM_SP_STATUS_EG_BABBLE_PACKET            ((uint32_t)0x00000001)
#define TSI721_PBM_SP_STATUS_EG_BAD_CHANNEL              ((uint32_t)0x00000002)
#define TSI721_PBM_SP_STATUS_EG_CRQ_OVERFLOW             ((uint32_t)0x00000008)
#define TSI721_PBM_SP_STATUS_EG_DATA_OVERFLOW            ((uint32_t)0x00000010)
#define TSI721_PBM_SP_STATUS_EG_DNFL_FATAL               ((uint32_t)0x00000020)
#define TSI721_PBM_SP_STATUS_EG_DNFL_COR                 ((uint32_t)0x00000040)
#define TSI721_PBM_SP_STATUS_EG_DOH_FATAL                ((uint32_t)0x00000080)
#define TSI721_PBM_SP_STATUS_EG_DOH_COR                  ((uint32_t)0x00000100)
#define TSI721_PBM_SP_STATUS_EG_DATA_UNCOR               ((uint32_t)0x00000800)
#define TSI721_PBM_SP_STATUS_EG_DATA_COR                 ((uint32_t)0x00001000)
#define TSI721_PBM_SP_STATUS_EG_EMPTY                    ((uint32_t)0x00008000)
#define TSI721_PBM_SP_STATUS_IG_EMPTY                    ((uint32_t)0x00010000)
#define TSI721_PBM_SP_STATUS_IG_DNFL_FATAL               ((uint32_t)0x00400000)
#define TSI721_PBM_SP_STATUS_IG_DNFL_COR                 ((uint32_t)0x00800000)
#define TSI721_PBM_SP_STATUS_IG_DOH_FATAL                ((uint32_t)0x01000000)
#define TSI721_PBM_SP_STATUS_IG_DOH_COR                  ((uint32_t)0x02000000)
#define TSI721_PBM_SP_STATUS_IG_TFL_FATAL                ((uint32_t)0x04000000)
#define TSI721_PBM_SP_STATUS_IG_TFL_COR                  ((uint32_t)0x08000000)
#define TSI721_PBM_SP_STATUS_IG_TAG_FATAL                ((uint32_t)0x10000000)
#define TSI721_PBM_SP_STATUS_IG_TAG_COR                  ((uint32_t)0x20000000)
#define TSI721_PBM_SP_STATUS_IG_DATA_UNCOR               ((uint32_t)0x40000000)
#define TSI721_PBM_SP_STATUS_IG_DATA_COR                 ((uint32_t)0x80000000)

/* TSI721_PBM_SP_INT_ENABLE : Register Bits Masks Definitions */
#define TSI721_PBM_SP_INT_ENABLE_EG_BABBLE_PACKET        ((uint32_t)0x00000001)
#define TSI721_PBM_SP_INT_ENABLE_EG_BAD_CHANNEL          ((uint32_t)0x00000002)
#define TSI721_PBM_SP_INT_ENABLE_EG_CRQ_OVERFLOW         ((uint32_t)0x00000008)
#define TSI721_PBM_SP_INT_ENABLE_EG_DATA_OVERFLOW        ((uint32_t)0x00000010)
#define TSI721_PBM_SP_INT_ENABLE_EG_DNFL_FATAL           ((uint32_t)0x00000020)
#define TSI721_PBM_SP_INT_ENABLE_EG_DNFL_COR             ((uint32_t)0x00000040)
#define TSI721_PBM_SP_INT_ENABLE_EG_DOH_FATAL            ((uint32_t)0x00000080)
#define TSI721_PBM_SP_INT_ENABLE_EG_DOH_COR              ((uint32_t)0x00000100)
#define TSI721_PBM_SP_INT_ENABLE_EG_DATA_UNCOR           ((uint32_t)0x00000800)
#define TSI721_PBM_SP_INT_ENABLE_EG_DATA_COR             ((uint32_t)0x00001000)
#define TSI721_PBM_SP_INT_ENABLE_IG_DNFL_FATAL           ((uint32_t)0x00400000)
#define TSI721_PBM_SP_INT_ENABLE_IG_DNFL_COR             ((uint32_t)0x00800000)
#define TSI721_PBM_SP_INT_ENABLE_IG_DOH_FATAL            ((uint32_t)0x01000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_DOH_COR              ((uint32_t)0x02000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_TFL_FATAL            ((uint32_t)0x04000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_TFL_COR              ((uint32_t)0x08000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_TAG_FATAL            ((uint32_t)0x10000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_TAG_COR              ((uint32_t)0x20000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_DATA_UNCOR           ((uint32_t)0x40000000)
#define TSI721_PBM_SP_INT_ENABLE_IG_DATA_COR             ((uint32_t)0x80000000)

/* TSI721_PBM_SP_PW_ENABLE : Register Bits Masks Definitions */
#define TSI721_PBM_SP_PW_ENABLE_EG_BABBLE_PACKET         ((uint32_t)0x00000001)
#define TSI721_PBM_SP_PW_ENABLE_EG_BAD_CHANNEL           ((uint32_t)0x00000002)
#define TSI721_PBM_SP_PW_ENABLE_EG_CRQ_OVERFLOW          ((uint32_t)0x00000008)
#define TSI721_PBM_SP_PW_ENABLE_EG_DATA_OVERFLOW         ((uint32_t)0x00000010)
#define TSI721_PBM_SP_PW_ENABLE_EG_DNFL_FATAL            ((uint32_t)0x00000020)
#define TSI721_PBM_SP_PW_ENABLE_EG_DNFL_COR              ((uint32_t)0x00000040)
#define TSI721_PBM_SP_PW_ENABLE_EG_DOH_FATAL             ((uint32_t)0x00000080)
#define TSI721_PBM_SP_PW_ENABLE_EG_DOH_COR               ((uint32_t)0x00000100)
#define TSI721_PBM_SP_PW_ENABLE_EG_DATA_UNCOR            ((uint32_t)0x00000800)
#define TSI721_PBM_SP_PW_ENABLE_EG_DATA_COR              ((uint32_t)0x00001000)
#define TSI721_PBM_SP_PW_ENABLE_IG_DNFL_FATAL            ((uint32_t)0x00400000)
#define TSI721_PBM_SP_PW_ENABLE_IG_DNFL_COR              ((uint32_t)0x00800000)
#define TSI721_PBM_SP_PW_ENABLE_IG_DOH_FATAL             ((uint32_t)0x01000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_DOH_COR               ((uint32_t)0x02000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_TFL_FATAL             ((uint32_t)0x04000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_TFL_COR               ((uint32_t)0x08000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_TAG_FATAL             ((uint32_t)0x10000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_TAG_COR               ((uint32_t)0x20000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_DATA_UNCOR            ((uint32_t)0x40000000)
#define TSI721_PBM_SP_PW_ENABLE_IG_DATA_COR              ((uint32_t)0x80000000)

/* TSI721_PBM_SP_EVENT_GEN : Register Bits Masks Definitions */
#define TSI721_PBM_SP_EVENT_GEN_EG_BABBLE_PACKET         ((uint32_t)0x00000001)
#define TSI721_PBM_SP_EVENT_GEN_EG_BAD_CHANNEL           ((uint32_t)0x00000002)
#define TSI721_PBM_SP_EVENT_GEN_EG_CRQ_OVERFLOW          ((uint32_t)0x00000008)
#define TSI721_PBM_SP_EVENT_GEN_EG_DATA_OVERFLOW         ((uint32_t)0x00000010)
#define TSI721_PBM_SP_EVENT_GEN_EG_DNFL_FATAL            ((uint32_t)0x00000020)
#define TSI721_PBM_SP_EVENT_GEN_EG_DNFL_COR              ((uint32_t)0x00000040)
#define TSI721_PBM_SP_EVENT_GEN_EG_DOH_FATAL             ((uint32_t)0x00000080)
#define TSI721_PBM_SP_EVENT_GEN_EG_DOH_COR               ((uint32_t)0x00000100)
#define TSI721_PBM_SP_EVENT_GEN_EG_DATA_UNCOR            ((uint32_t)0x00000800)
#define TSI721_PBM_SP_EVENT_GEN_EG_DATA_COR              ((uint32_t)0x00001000)
#define TSI721_PBM_SP_EVENT_GEN_IG_DNFL_FATAL            ((uint32_t)0x00400000)
#define TSI721_PBM_SP_EVENT_GEN_IG_DNFL_COR              ((uint32_t)0x00800000)
#define TSI721_PBM_SP_EVENT_GEN_IG_DOH_FATAL             ((uint32_t)0x01000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_DOH_COR               ((uint32_t)0x02000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_TFL_FATAL             ((uint32_t)0x04000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_TFL_COR               ((uint32_t)0x08000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_TAG_FATAL             ((uint32_t)0x10000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_TAG_COR               ((uint32_t)0x20000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_DATA_UNCOR            ((uint32_t)0x40000000)
#define TSI721_PBM_SP_EVENT_GEN_IG_DATA_COR              ((uint32_t)0x80000000)

/* TSI721_PBM_SP_IG_WATERMARK0 : Register Bits Masks Definitions */
#define TSI721_PBM_SP_IG_WATERMARK0_PRIO0_WM             ((uint32_t)0x000003ff)
#define TSI721_PBM_SP_IG_WATERMARK0_PRIO0CRF_WM          ((uint32_t)0x03ff0000)

/* TSI721_PBM_SP_IG_WATERMARK1 : Register Bits Masks Definitions */
#define TSI721_PBM_SP_IG_WATERMARK1_PRIO1_WM             ((uint32_t)0x000003ff)
#define TSI721_PBM_SP_IG_WATERMARK1_PRIO1CRF_WM          ((uint32_t)0x03ff0000)

/* TSI721_PBM_SP_IG_WATERMARK2 : Register Bits Masks Definitions */
#define TSI721_PBM_SP_IG_WATERMARK2_PRIO2_WM             ((uint32_t)0x000003ff)
#define TSI721_PBM_SP_IG_WATERMARK2_PRIO2CRF_WM          ((uint32_t)0x03ff0000)

/* TSI721_PBM_SP_IG_WATERMARK3 : Register Bits Masks Definitions */
#define TSI721_PBM_SP_IG_WATERMARK3_PRIO3_WM             ((uint32_t)0x000003ff)
#define TSI721_PBM_SP_IG_WATERMARK3_PRIO3CRF_WM          ((uint32_t)0x03ff0000)

/* TSI721_EM_BH : Register Bits Masks Definitions */
#define TSI721_EM_BH_BLK_TYPE                            ((uint32_t)0x00000fff)
#define TSI721_EM_BH_BLK_REV                             ((uint32_t)0x0000f000)
#define TSI721_EM_BH_NEXT_BLK_PTR                        ((uint32_t)0xffff0000)

/* TSI721_EM_INT_STAT : Register Bits Masks Definitions */
#define TSI721_EM_INT_STAT_LOCALOG                       ((uint32_t)0x00000100)
#define TSI721_EM_INT_STAT_IG_DATA_UNCOR                 ((uint32_t)0x00000400)
#define TSI721_EM_INT_STAT_IG_DATA_COR                   ((uint32_t)0x00000800)
#define TSI721_EM_INT_STAT_PW_RX                         ((uint32_t)0x00010000)
#define TSI721_EM_INT_STAT_MECS                          ((uint32_t)0x04000000)
#define TSI721_EM_INT_STAT_RCS                           ((uint32_t)0x08000000)
#define TSI721_EM_INT_STAT_LOG                           ((uint32_t)0x10000000)
#define TSI721_EM_INT_STAT_PORT                          ((uint32_t)0x20000000)

/* TSI721_EM_INT_ENABLE : Register Bits Masks Definitions */
#define TSI721_EM_INT_ENABLE_LOCALOG                     ((uint32_t)0x00000100)
#define TSI721_EM_INT_ENABLE_IG_DATA_UNCOR               ((uint32_t)0x00000400)
#define TSI721_EM_INT_ENABLE_IG_DATA_COR                 ((uint32_t)0x00000800)
#define TSI721_EM_INT_ENABLE_PW_RX                       ((uint32_t)0x00010000)
#define TSI721_EM_INT_ENABLE_MECS                        ((uint32_t)0x04000000)
#define TSI721_EM_INT_ENABLE_LOG                         ((uint32_t)0x10000000)

/* TSI721_EM_INT_PORT_STAT : Register Bits Masks Definitions */
#define TSI721_EM_INT_PORT_STAT_IRQ_PENDING              ((uint32_t)0x00000001)

/* TSI721_EM_PW_STAT : Register Bits Masks Definitions */
#define TSI721_EM_PW_STAT_LOCALOG                        ((uint32_t)0x00000100)
#define TSI721_EM_PW_STAT_IG_DATA_UNCOR                  ((uint32_t)0x00000400)
#define TSI721_EM_PW_STAT_IG_DATA_COR                    ((uint32_t)0x00000800)
#define TSI721_EM_PW_STAT_RCS                            ((uint32_t)0x08000000)
#define TSI721_EM_PW_STAT_LOG                            ((uint32_t)0x10000000)
#define TSI721_EM_PW_STAT_PORT                           ((uint32_t)0x20000000)

/* TSI721_EM_PW_ENABLE : Register Bits Masks Definitions */
#define TSI721_EM_PW_ENABLE_LOCALOG                      ((uint32_t)0x00000100)
#define TSI721_EM_PW_ENABLE_IG_DATA_UNCOR                ((uint32_t)0x00000400)
#define TSI721_EM_PW_ENABLE_IG_DATA_COR                  ((uint32_t)0x00000800)
#define TSI721_EM_PW_ENABLE_LOG                          ((uint32_t)0x10000000)

/* TSI721_EM_PW_PORT_STAT : Register Bits Masks Definitions */
#define TSI721_EM_PW_PORT_STAT_PW_PENDING                ((uint32_t)0x00000001)

/* TSI721_EM_DEV_INT_EN : Register Bits Masks Definitions */
#define TSI721_EM_DEV_INT_EN_INT_EN                      ((uint32_t)0x00000001)

/* TSI721_EM_DEV_PW_EN : Register Bits Masks Definitions */
#define TSI721_EM_DEV_PW_EN_PW_EN                        ((uint32_t)0x00000001)

/* TSI721_EM_MECS_STAT : Register Bits Masks Definitions */
#define TSI721_EM_MECS_STAT_CMD_STAT                     ((uint32_t)0x000000ff)

/* TSI721_EM_MECS_INT_EN : Register Bits Masks Definitions */
#define TSI721_EM_MECS_INT_EN_CMD_EN                     ((uint32_t)0x000000ff)

/* TSI721_EM_MECS_CAP_EN : Register Bits Masks Definitions */
#define TSI721_EM_MECS_CAP_EN_CMD_EN                     ((uint32_t)0x000000ff)

/* TSI721_EM_MECS_TRIG_EN : Register Bits Masks Definitions */
#define TSI721_EM_MECS_TRIG_EN_CMD_EN                    ((uint32_t)0x000000ff)
#define TSI721_EM_MECS_TRIG_EN_CMD_STAT                  ((uint32_t)0x0000ff00)

/* TSI721_EM_MECS_REQ : Register Bits Masks Definitions */
#define TSI721_EM_MECS_REQ_CMD                           ((uint32_t)0x000000ff)
#define TSI721_EM_MECS_REQ_SEND                          ((uint32_t)0x00000100)

/* TSI721_EM_MECS_PORT_STAT : Register Bits Masks Definitions */
#define TSI721_EM_MECS_PORT_STAT_PORT                    ((uint32_t)0x00000001)

/* TSI721_EM_MECS_EVENT_GEN : Register Bits Masks Definitions */
#define TSI721_EM_MECS_EVENT_GEN_CMD_STAT                ((uint32_t)0x000000ff)
#define TSI721_EM_MECS_EVENT_GEN_IG_DATA_UNCOR           ((uint32_t)0x00000400)
#define TSI721_EM_MECS_EVENT_GEN_IG_DATA_COR             ((uint32_t)0x00000800)

/* TSI721_EM_RST_PORT_STAT : Register Bits Masks Definitions */
#define TSI721_EM_RST_PORT_STAT_RST_REQ                  ((uint32_t)0x00000001)

/* TSI721_EM_RST_INT_EN : Register Bits Masks Definitions */
#define TSI721_EM_RST_INT_EN_RST_INT_EN                  ((uint32_t)0x00000001)

/* TSI721_EM_RST_PW_EN : Register Bits Masks Definitions */
#define TSI721_EM_RST_PW_EN_RST_PW_EN                    ((uint32_t)0x00000001)

/* TSI721_PW_BH : Register Bits Masks Definitions */
#define TSI721_PW_BH_BLK_TYPE                            ((uint32_t)0x00000fff)
#define TSI721_PW_BH_BLK_REV                             ((uint32_t)0x0000f000)
#define TSI721_PW_BH_NEXT_BLK_PTR                        ((uint32_t)0xffff0000)

/* TSI721_PW_CTL : Register Bits Masks Definitions */
#define TSI721_PW_CTL_PWC_MODE                           ((uint32_t)0x01000000)
#define TSI721_PW_CTL_PW_TIMER                           ((uint32_t)0xf0000000)
#define TSI721_PW_CTL_PW_TIMER_103US                     ((uint32_t)0x10000000)
#define TSI721_PW_CTL_PW_TIMER_205US                     ((uint32_t)0x20000000)
#define TSI721_PW_CTL_PW_TIMER_410US                     ((uint32_t)0x40000000)
#define TSI721_PW_CTL_PW_TIMER_820US                     ((uint32_t)0x80000000)

/* TSI721_PW_ROUTE : Register Bits Masks Definitions */
#define TSI721_PW_ROUTE_PORT                             ((uint32_t)0x00000001)

/* TSI721_PW_RX_STAT : Register Bits Masks Definitions */
#define TSI721_PW_RX_STAT_PW_VAL                         ((uint32_t)0x00000001)
#define TSI721_PW_RX_STAT_PW_DISC                        ((uint32_t)0x00000002)
#define TSI721_PW_RX_STAT_PW_TRUNC                       ((uint32_t)0x00000004)
#define TSI721_PW_RX_STAT_PW_SHORT                       ((uint32_t)0x00000008)
#define TSI721_PW_RX_STAT_WDPTR                          ((uint32_t)0x00000100)
#define TSI721_PW_RX_STAT_WR_SIZE                        ((uint32_t)0x0000f000)

/* TSI721_PW_RX_EVENT_GEN : Register Bits Masks Definitions */
#define TSI721_PW_RX_EVENT_GEN_PW_VAL                    ((uint32_t)0x00000001)
#define TSI721_PW_RX_EVENT_GEN_PW_DISC                   ((uint32_t)0x00000002)

/* TSI721_PW_RX_CAPT : Register Bits Masks Definitions */
#define TSI721_PW_RX_CAPT_PW_CAPT                        ((uint32_t)0xffffffff)

/* TSI721_LLM_BH : Register Bits Masks Definitions */
#define TSI721_LLM_BH_BLK_TYPE                           ((uint32_t)0x00000fff)
#define TSI721_LLM_BH_BLK_REV                            ((uint32_t)0x0000f000)
#define TSI721_LLM_BH_NEXT_BLK_PTR                       ((uint32_t)0xffff0000)

/* TSI721_MTC_WR_RESTRICT : Register Bits Masks Definitions */
#define TSI721_MTC_WR_RESTRICT_WR_DIS                    ((uint32_t)0x00000001)

/* TSI721_MTC_PWR_RESTRICT : Register Bits Masks Definitions */
#define TSI721_MTC_PWR_RESTRICT_PWR_DIS                  ((uint32_t)0x00000001)

/* TSI721_MTC_RD_RESTRICT : Register Bits Masks Definitions */
#define TSI721_MTC_RD_RESTRICT_RD_DIS                    ((uint32_t)0x00000001)

/* TSI721_WHITEBOARD : Register Bits Masks Definitions */
#define TSI721_WHITEBOARD_SCRATCH                        ((uint32_t)0xffffffff)

/* TSI721_PRESCALAR_SRV_CLK : Register Bits Masks Definitions */
#define TSI721_PRESCALAR_SRV_CLK_PRESCALAR_SRV_CLK       ((uint32_t)0x000000ff)

/* TSI721_REG_RST_CTL : Register Bits Masks Definitions */
#define TSI721_REG_RST_CTL_CLEAR_STICKY                  ((uint32_t)0x00000001)

/* TSI721_LOCAL_ERR_DET : Register Bits Masks Definitions */
#define TSI721_LOCAL_ERR_DET_ILL_TYPE                    ((uint32_t)0x00400000)
#define TSI721_LOCAL_ERR_DET_ILL_ID                      ((uint32_t)0x04000000)

/* TSI721_LOCAL_ERR_EN : Register Bits Masks Definitions */
#define TSI721_LOCAL_ERR_EN_ILL_TYPE_EN                  ((uint32_t)0x00400000)
#define TSI721_LOCAL_ERR_EN_ILL_ID_EN                    ((uint32_t)0x04000000)

/* TSI721_LOCAL_H_ADDR_CAPT : Register Bits Masks Definitions */
#define TSI721_LOCAL_H_ADDR_CAPT_ADDR                    ((uint32_t)0xffffffff)

/* TSI721_LOCAL_ADDR_CAPT : Register Bits Masks Definitions */
#define TSI721_LOCAL_ADDR_CAPT_XAMSBS                    ((uint32_t)0x00000003)
#define TSI721_LOCAL_ADDR_CAPT_ADDR                      ((uint32_t)0xfffffff8)

/* TSI721_LOCAL_ID_CAPT : Register Bits Masks Definitions */
#define TSI721_LOCAL_ID_CAPT_SRC_ID                      ((uint32_t)0x000000ff)
#define TSI721_LOCAL_ID_CAPT_MSB_SRC_ID                  ((uint32_t)0x0000ff00)
#define TSI721_LOCAL_ID_CAPT_DEST_ID                     ((uint32_t)0x00ff0000)
#define TSI721_LOCAL_ID_CAPT_MSB_DEST_ID                 ((uint32_t)0xff000000)

/* TSI721_LOCAL_CTRL_CAPT : Register Bits Masks Definitions */
#define TSI721_LOCAL_CTRL_CAPT_MESSAGE_INFO              ((uint32_t)0x00ff0000)
#define TSI721_LOCAL_CTRL_CAPT_TTYPE                     ((uint32_t)0x0f000000)
#define TSI721_LOCAL_CTRL_CAPT_FTYPE                     ((uint32_t)0xf0000000)

/* TSI721_FABRIC_BH : Register Bits Masks Definitions */
#define TSI721_FABRIC_BH_BLK_TYPE                        ((uint32_t)0x00000fff)
#define TSI721_FABRIC_BH_BLK_REV                         ((uint32_t)0x0000f000)
#define TSI721_FABRIC_BH_NEXT_BLK_PTR                    ((uint32_t)0xffff0000)

/* TSI721_PRBS_BH : Register Bits Masks Definitions */
#define TSI721_PRBS_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define TSI721_PRBS_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define TSI721_PRBS_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* TSI721_PRBS_LANEX_CTRL : Register Bits Masks Definitions */
#define TSI721_PRBS_LANEX_CTRL_TRANSMIT                  ((uint32_t)0x01000000)
#define TSI721_PRBS_LANEX_CTRL_ENABLE                    ((uint32_t)0x02000000)
#define TSI721_PRBS_LANEX_CTRL_TRAIN                     ((uint32_t)0x04000000)
#define TSI721_PRBS_LANEX_CTRL_UNI                       ((uint32_t)0x08000000)
#define TSI721_PRBS_LANEX_CTRL_PATTERN                   ((uint32_t)0xf0000000)

/* TSI721_PRBS_LANEX_SEED : Register Bits Masks Definitions */
#define TSI721_PRBS_LANEX_SEED_SEED                      ((uint32_t)0xffffffff)

/* TSI721_PRBS_LANEX_ERR_COUNT : Register Bits Masks Definitions */
#define TSI721_PRBS_LANEX_ERR_COUNT_COUNT                ((uint32_t)0xffffffff)


/**********************************************************/
/* TSI721 : Mapping Register address offset definitions   */
/**********************************************************/

#define TSI721_IDQ_CTLX(X)                   ((uint32_t)(0x20000 + 0x1000*(X)))
#define TSI721_IDQ_STSX(X)                   ((uint32_t)(0x20004 + 0x1000*(X)))
#define TSI721_IDQ_MASKX(X)                  ((uint32_t)(0x20008 + 0x1000*(X)))
#define TSI721_IDQ_RPX(X)                    ((uint32_t)(0x2000c + 0x1000*(X)))
#define TSI721_IDQ_WPX(X)                    ((uint32_t)(0x20010 + 0x1000*(X)))
#define TSI721_IDQ_BASELX(X)                 ((uint32_t)(0x20014 + 0x1000*(X)))
#define TSI721_IDQ_BASEUX(X)                 ((uint32_t)(0x20018 + 0x1000*(X)))
#define TSI721_IDQ_SIZEX(X)                  ((uint32_t)(0x2001c + 0x1000*(X)))
#define TSI721_SR_CHXINT(X)                  ((uint32_t)(0x20040 + 0x1000*(X)))
#define TSI721_SR_CHXINTE(X)                 ((uint32_t)(0x20044 + 0x1000*(X)))
#define TSI721_SR_CHXINTSET(X)               ((uint32_t)(0x20048 + 0x1000*(X)))
#define TSI721_ODB_CNTX(X)                   ((uint32_t)(0x20100 + 0x1000*(X)))
#define TSI721_ODB_LOG_DAT0X(X)              ((uint32_t)(0x20104 + 0x1000*(X)))
#define TSI721_ODB_LOG_DAT1X(X)              ((uint32_t)(0x20108 + 0x1000*(X)))
#define TSI721_ODB_LOG_DAT2X(X)              ((uint32_t)(0x2010c + 0x1000*(X)))
#define TSI721_ODB_LOG_DAT3X(X)              ((uint32_t)(0x20120 + 0x1000*(X)))
#define TSI721_ODBXLOGSTS(X)                 ((uint32_t)(0x20124 + 0x1000*(X)))
#define TSI721_IBWIN_LBX(X)                   ((uint32_t)(0x29000 + 0x020*(X)))
#define TSI721_IBWIN_UBX(X)                   ((uint32_t)(0x29004 + 0x020*(X)))
#define TSI721_IBWIN_SZX(X)                   ((uint32_t)(0x29008 + 0x020*(X)))
#define TSI721_IBWIN_TLAX(X)                  ((uint32_t)(0x2900c + 0x020*(X)))
#define TSI721_IBWIN_TUAX(X)                  ((uint32_t)(0x29010 + 0x020*(X)))
#define TSI721_SR2PC_GEN_INTE                            ((uint32_t)0x00029800)
#define TSI721_SR2PC_PWE                                 ((uint32_t)0x00029804)
#define TSI721_SR2PC_GEN_INT                             ((uint32_t)0x00029808)
#define TSI721_SR2PC_GEN_INTSET                          ((uint32_t)0x0002980c)
#define TSI721_SR2PC_CORR_ECC_LOG                        ((uint32_t)0x00029810)
#define TSI721_SR2PC_UNCORR_ECC_LOG                      ((uint32_t)0x00029814)
#define TSI721_SR2PC_PCIE_PS                             ((uint32_t)0x00029820)
#define TSI721_LOGBUF_STS                                ((uint32_t)0x00029824)
#define TSI721_DEV_INTE                                  ((uint32_t)0x00029840)
#define TSI721_DEV_INT                                   ((uint32_t)0x00029844)
#define TSI721_DEV_CHAN_INTE                             ((uint32_t)0x0002984c)
#define TSI721_DEV_CHAN_INT                              ((uint32_t)0x00029850)
#define TSI721_INT_MOD                                   ((uint32_t)0x00029858)
#define TSI721_RXPKT_SMSG_CNT                            ((uint32_t)0x00029900)
#define TSI721_RXRSP_BDMA_CNT                            ((uint32_t)0x00029904)
#define TSI721_RXPKT_BRG_CNT                             ((uint32_t)0x00029908)
#define TSI721_TXTLP_SMSG_CNT                            ((uint32_t)0x0002990c)
#define TSI721_TXTLP_BDMA_CNT                            ((uint32_t)0x00029910)
#define TSI721_TXTLP_BRG_CNT                             ((uint32_t)0x00029914)
#define TSI721_BRG_PKT_ERR_CNT                           ((uint32_t)0x0002991c)
#define TSI721_MWR_CNT                                   ((uint32_t)0x00029a00)
#define TSI721_NWR_CNT                                   ((uint32_t)0x00029a04)
#define TSI721_MWR_LOG_DAT0                              ((uint32_t)0x00029a08)
#define TSI721_MWR_LOG_DAT1                              ((uint32_t)0x00029a0c)
#define TSI721_MWR_LOG_DAT2                              ((uint32_t)0x00029a10)
#define TSI721_MWR_LOG_DAT3                              ((uint32_t)0x00029a14)
#define TSI721_NWR_LOG_DAT0                              ((uint32_t)0x00029a18)
#define TSI721_NWR_LOG_DAT1                              ((uint32_t)0x00029a1c)
#define TSI721_NWR_LOG_DAT2                              ((uint32_t)0x00029a20)
#define TSI721_NWR_LOG_DAT3                              ((uint32_t)0x00029a24)
#define TSI721_MSIX_PBAL                                 ((uint32_t)0x0002a000)
#define TSI721_MSIX_PBAM                                 ((uint32_t)0x0002a004)
#define TSI721_MSIX_PBAU                                 ((uint32_t)0x0002a008)
#define TSI721_MSIX_TAB_ADDRLX(X)             ((uint32_t)(0x2c000 + 0x010*(X)))
#define TSI721_MSIX_TAB_ADDRUX(X)             ((uint32_t)(0x2c004 + 0x010*(X)))
#define TSI721_MSIX_TAB_DATAX(X)              ((uint32_t)(0x2c008 + 0x010*(X)))
#define TSI721_MSIX_TAB_MSKX(X)               ((uint32_t)(0x2c00c + 0x010*(X)))
#define TSI721_OBWINLBX(X)                    ((uint32_t)(0x40000 + 0x020*(X)))
#define TSI721_OBWINUBX(X)                    ((uint32_t)(0x40004 + 0x020*(X)))
#define TSI721_OBWINSZX(X)                    ((uint32_t)(0x40008 + 0x020*(X)))
#define TSI721_ZONE_SEL                                  ((uint32_t)0x00041300)
#define TSI721_LUT_DATA0                                 ((uint32_t)0x00041304)
#define TSI721_LUT_DATA1                                 ((uint32_t)0x00041308)
#define TSI721_LUT_DATA2                                 ((uint32_t)0x0004130c)
#define TSI721_PC2SR_INTE                                ((uint32_t)0x00041310)
#define TSI721_PC2SR_INT                                 ((uint32_t)0x00041314)
#define TSI721_PC2SR_INTSET                              ((uint32_t)0x00041318)
#define TSI721_PC2SR_ECC_LOG                             ((uint32_t)0x0004131c)
#define TSI721_CPL_SMSG_CNT                              ((uint32_t)0x00041404)
#define TSI721_CPL_BDMA_CNT                              ((uint32_t)0x00041408)
#define TSI721_RXTLP_BRG_CNT                             ((uint32_t)0x0004140c)
#define TSI721_TXPKT_SMSG_CNT                            ((uint32_t)0x00041410)
#define TSI721_TXPKT_BDMA_CNT                            ((uint32_t)0x00041414)
#define TSI721_TXPKT_BRG_CNT                             ((uint32_t)0x00041418)
#define TSI721_BRG_TLP_ERR_CNT                           ((uint32_t)0x0004141c)


/********************************************************/
/* TSI721 : Mapping Register Bit Masks and Reset Values */
/*           definitions for every register             */
/********************************************************/

/* TSI721_IDQ_CTLX : Register Bits Masks Definitions */
#define TSI721_IDQ_CTLX_INIT                             ((uint32_t)0x00000001)
#define TSI721_IDQ_CTLX_SUSPEND                          ((uint32_t)0x00000002)

/* TSI721_IDQ_STSX : Register Bits Masks Definitions */
#define TSI721_IDQ_STSX_RUN                              ((uint32_t)0x00200000)

/* TSI721_IDQ_MASKX : Register Bits Masks Definitions */
#define TSI721_IDQ_MASKX_PATTERN                         ((uint32_t)0x0000ffff)
#define TSI721_IDQ_MASKX_MASK                            ((uint32_t)0xffff0000)

/* TSI721_IDQ_RPX : Register Bits Masks Definitions */
#define TSI721_IDQ_RPX_RD_PTR                            ((uint32_t)0x0007ffff)

/* TSI721_IDQ_WPX : Register Bits Masks Definitions */
#define TSI721_IDQ_WPX_WR_PTR                            ((uint32_t)0x0007ffff)

/* TSI721_IDQ_BASELX : Register Bits Masks Definitions */
#define TSI721_IDQ_BASELX_ADD                            ((uint32_t)0xffffffc0)

/* TSI721_IDQ_BASEUX : Register Bits Masks Definitions */
#define TSI721_IDQ_BASEUX_ADD                            ((uint32_t)0xffffffff)

/* TSI721_IDQ_SIZEX : Register Bits Masks Definitions */
#define TSI721_IDQ_SIZEX_SIZE                            ((uint32_t)0x0000000f)

/* TSI721_SR_CHXINT : Register Bits Masks Definitions */
#define TSI721_SR_CHXINT_ODB_ERR                         ((uint32_t)0x00000001)
#define TSI721_SR_CHXINT_ODB_RETRY                       ((uint32_t)0x00000002)
#define TSI721_SR_CHXINT_ODB_TO                          ((uint32_t)0x00000004)
#define TSI721_SR_CHXINT_SUSPENDED                       ((uint32_t)0x00000008)
#define TSI721_SR_CHXINT_IDBQ_RCV                        ((uint32_t)0x00000010)
#define TSI721_SR_CHXINT_ODB_OK                          ((uint32_t)0x00000020)

/* TSI721_SR_CHXINTE : Register Bits Masks Definitions */
#define TSI721_SR_CHXINTE_ODB_ERR_EN                     ((uint32_t)0x00000001)
#define TSI721_SR_CHXINTE_ODB_RETRY_EN                   ((uint32_t)0x00000002)
#define TSI721_SR_CHXINTE_ODB_TO_EN                      ((uint32_t)0x00000004)
#define TSI721_SR_CHXINTE_SUSPENDED_EN                   ((uint32_t)0x00000008)
#define TSI721_SR_CHXINTE_IDBQ_RCV_EN                    ((uint32_t)0x00000010)
#define TSI721_SR_CHXINTE_ODB_OK_EN                      ((uint32_t)0x00000020)

/* TSI721_SR_CHXINTSET : Register Bits Masks Definitions */
#define TSI721_SR_CHXINTSET_ODB_ERR_SET                  ((uint32_t)0x00000001)
#define TSI721_SR_CHXINTSET_ODB_RETRY_SET                ((uint32_t)0x00000002)
#define TSI721_SR_CHXINTSET_ODB_TO_SET                   ((uint32_t)0x00000004)
#define TSI721_SR_CHXINTSET_SUSPENDED_SET                ((uint32_t)0x00000008)
#define TSI721_SR_CHXINTSET_IDBQ_RCV_SET                 ((uint32_t)0x00000010)
#define TSI721_SR_CHXINTSET_ODB_OK_SET                   ((uint32_t)0x00000020)

/* TSI721_ODB_CNTX : Register Bits Masks Definitions */
#define TSI721_ODB_CNTX_ODB_OK_CNT                       ((uint32_t)0x0000ffff)
#define TSI721_ODB_CNTX_ODB_TOT_CNT                      ((uint32_t)0xffff0000)

/* TSI721_ODB_LOG_DAT0X : Register Bits Masks Definitions */
#define TSI721_ODB_LOG_DAT0X_DATA                        ((uint32_t)0xffffffff)

/* TSI721_ODB_LOG_DAT1X : Register Bits Masks Definitions */
#define TSI721_ODB_LOG_DAT1X_DATA                        ((uint32_t)0xffffffff)

/* TSI721_ODB_LOG_DAT2X : Register Bits Masks Definitions */
#define TSI721_ODB_LOG_DAT2X_DATA                        ((uint32_t)0xffffffff)

/* TSI721_ODB_LOG_DAT3X : Register Bits Masks Definitions */
#define TSI721_ODB_LOG_DAT3X_DATA                        ((uint32_t)0xffffffff)

/* TSI721_ODBXLOGSTS : Register Bits Masks Definitions */
#define TSI721_ODBXLOGSTS_LOG_BUF_ERR                    ((uint32_t)0x00000003)

/* TSI721_IBWIN_LBX : Register Bits Masks Definitions */
#define TSI721_IBWIN_LBX_WIN_EN                          ((uint32_t)0x00000001)
#define TSI721_IBWIN_LBX_ADD                             ((uint32_t)0xfffff000)

/* TSI721_IBWIN_UBX : Register Bits Masks Definitions */
#define TSI721_IBWIN_UBX_ADD                             ((uint32_t)0xffffffff)

/* TSI721_IBWIN_SZX : Register Bits Masks Definitions */
#define TSI721_IBWIN_SZX_SIZE                            ((uint32_t)0x00001f00)
#define TSI721_IBWIN_SZX_ADD                             ((uint32_t)0x03000000)

/* TSI721_IBWIN_TLAX : Register Bits Masks Definitions */
#define TSI721_IBWIN_TLAX_ADD                            ((uint32_t)0xfffff000)

/* TSI721_IBWIN_TUAX : Register Bits Masks Definitions */
#define TSI721_IBWIN_TUAX_ADD                            ((uint32_t)0xffffffff)

/* TSI721_SR2PC_GEN_INTE : Register Bits Masks Definitions */
#define TSI721_SR2PC_GEN_INTE_MW_RSP_ERR_EN              ((uint32_t)0x00000001)
#define TSI721_SR2PC_GEN_INTE_NW_RSP_ERR_EN              ((uint32_t)0x00000002)
#define TSI721_SR2PC_GEN_INTE_MW_RSP_TO_EN               ((uint32_t)0x00000004)
#define TSI721_SR2PC_GEN_INTE_NW_RSP_TO_EN               ((uint32_t)0x00000008)
#define TSI721_SR2PC_GEN_INTE_DB_MISS_EN                 ((uint32_t)0x00000010)
#define TSI721_SR2PC_GEN_INTE_ECC_CORR_EN                ((uint32_t)0x00000020)
#define TSI721_SR2PC_GEN_INTE_ECC_UNCORR_EN              ((uint32_t)0x00000040)
#define TSI721_SR2PC_GEN_INTE_DL_DOWN_EN                 ((uint32_t)0x00000100)
#define TSI721_SR2PC_GEN_INTE_MW_RSP_OK_EN               ((uint32_t)0x00000200)
#define TSI721_SR2PC_GEN_INTE_NW_RSP_OK_EN               ((uint32_t)0x00000400)
#define TSI721_SR2PC_GEN_INTE_UNS_RSP_EN                 ((uint32_t)0x00800000)
#define TSI721_SR2PC_GEN_INTE_RSP_TO_EN                  ((uint32_t)0x01000000)
#define TSI721_SR2PC_GEN_INTE_ILL_TARGET_EN              ((uint32_t)0x04000000)
#define TSI721_SR2PC_GEN_INTE_ILL_DEC_EN                 ((uint32_t)0x08000000)
#define TSI721_SR2PC_GEN_INTE_ERR_RSP_EN                 ((uint32_t)0x80000000)

/* TSI721_SR2PC_PWE : Register Bits Masks Definitions */
#define TSI721_SR2PC_PWE_DL_DOWN_EN                      ((uint32_t)0x00000100)
#define TSI721_SR2PC_PWE_UNS_RSP_EN                      ((uint32_t)0x00800000)
#define TSI721_SR2PC_PWE_RSP_TO_EN                       ((uint32_t)0x01000000)
#define TSI721_SR2PC_PWE_ILL_TARGET_EN                   ((uint32_t)0x04000000)
#define TSI721_SR2PC_PWE_ILL_DEC_EN                      ((uint32_t)0x08000000)
#define TSI721_SR2PC_PWE_ERR_RSP_EN                      ((uint32_t)0x80000000)

/* TSI721_SR2PC_GEN_INT : Register Bits Masks Definitions */
#define TSI721_SR2PC_GEN_INT_MW_RSP_ERR                  ((uint32_t)0x00000001)
#define TSI721_SR2PC_GEN_INT_NW_RSP_ERR                  ((uint32_t)0x00000002)
#define TSI721_SR2PC_GEN_INT_MW_RSP_TO                   ((uint32_t)0x00000004)
#define TSI721_SR2PC_GEN_INT_NW_RSP_TO                   ((uint32_t)0x00000008)
#define TSI721_SR2PC_GEN_INT_DB_MISS                     ((uint32_t)0x00000010)
#define TSI721_SR2PC_GEN_INT_ECC_CORR                    ((uint32_t)0x00000020)
#define TSI721_SR2PC_GEN_INT_ECC_UNCORR                  ((uint32_t)0x00000040)
#define TSI721_SR2PC_GEN_INT_DL_DOWN                     ((uint32_t)0x00000100)
#define TSI721_SR2PC_GEN_INT_NW_RSP_OK                   ((uint32_t)0x00000200)
#define TSI721_SR2PC_GEN_INT_MW_RSP_OK                   ((uint32_t)0x00000400)
#define TSI721_SR2PC_GEN_INT_UNS_RSP                     ((uint32_t)0x00800000)
#define TSI721_SR2PC_GEN_INT_RSP_TO                      ((uint32_t)0x01000000)
#define TSI721_SR2PC_GEN_INT_ILL_TARGET                  ((uint32_t)0x04000000)
#define TSI721_SR2PC_GEN_INT_ILL_DEC                     ((uint32_t)0x08000000)
#define TSI721_SR2PC_GEN_INT_ERR_RSP                     ((uint32_t)0x80000000)

/* TSI721_SR2PC_GEN_INTSET : Register Bits Masks Definitions */
#define TSI721_SR2PC_GEN_INTSET_MW_RSP_ERR_SET           ((uint32_t)0x00000001)
#define TSI721_SR2PC_GEN_INTSET_NW_RSP_ERR_SET           ((uint32_t)0x00000002)
#define TSI721_SR2PC_GEN_INTSET_MW_RSP_TO_SET            ((uint32_t)0x00000004)
#define TSI721_SR2PC_GEN_INTSET_NW_RSP_TO_SET            ((uint32_t)0x00000008)
#define TSI721_SR2PC_GEN_INTSET_DB_MISS_SET              ((uint32_t)0x00000010)
#define TSI721_SR2PC_GEN_INTSET_ECC_CORR_SET             ((uint32_t)0x00000020)
#define TSI721_SR2PC_GEN_INTSET_ECC_UNCORR_SET           ((uint32_t)0x00000040)
#define TSI721_SR2PC_GEN_INTSET_NW_RSP_OK_SET            ((uint32_t)0x00000200)
#define TSI721_SR2PC_GEN_INTSET_MW_RSP_OK_SET            ((uint32_t)0x00000400)
#define TSI721_SR2PC_GEN_INTSET_UNS_RSP_SET              ((uint32_t)0x00800000)
#define TSI721_SR2PC_GEN_INTSET_RSP_TO_SET               ((uint32_t)0x01000000)
#define TSI721_SR2PC_GEN_INTSET_ILL_TARGET_SET           ((uint32_t)0x04000000)
#define TSI721_SR2PC_GEN_INTSET_ILL_DEC_SET              ((uint32_t)0x08000000)
#define TSI721_SR2PC_GEN_INTSET_ERR_RSP_SET              ((uint32_t)0x80000000)

/* TSI721_SR2PC_CORR_ECC_LOG : Register Bits Masks Definitions */
#define TSI721_SR2PC_CORR_ECC_LOG_ECC_CORR_MEM           ((uint32_t)0x0007ffff)

/* TSI721_SR2PC_UNCORR_ECC_LOG : Register Bits Masks Definitions */
#define TSI721_SR2PC_UNCORR_ECC_LOG_ECC_UNCORR_MEM       ((uint32_t)0x0007ffff)

/* TSI721_SR2PC_PCIE_PS : Register Bits Masks Definitions */
#define TSI721_SR2PC_PCIE_PS_DSTATE                      ((uint32_t)0x00000003)

/* TSI721_LOGBUF_STS : Register Bits Masks Definitions */
#define TSI721_LOGBUF_STS_NWR_LOG_BUF_ERR                ((uint32_t)0x00000001)
#define TSI721_LOGBUF_STS_MWR_LOG_BUF_ERR                ((uint32_t)0x00000002)

/* TSI721_DEV_INTE : Register Bits Masks Definitions */
#define TSI721_DEV_INTE_INT_PC2SR_EN                     ((uint32_t)0x00000004)
#define TSI721_DEV_INTE_INT_I2C_EN                       ((uint32_t)0x00000010)
#define TSI721_DEV_INTE_INT_SRIO_EN                      ((uint32_t)0x00000020)
#define TSI721_DEV_INTE_INT_SR2PC_NONCH_EN               ((uint32_t)0x00000100)
#define TSI721_DEV_INTE_INT_SR2PC_CH_EN                  ((uint32_t)0x00000200)
#define TSI721_DEV_INTE_INT_SMSG_NONCH_EN                ((uint32_t)0x00000400)
#define TSI721_DEV_INTE_INT_SMSG_CH_EN                   ((uint32_t)0x00000800)
#define TSI721_DEV_INTE_INT_BDMA_NONCH_EN                ((uint32_t)0x00001000)
#define TSI721_DEV_INTE_INT_BDMA_CH_EN                   ((uint32_t)0x00002000)

/* TSI721_DEV_INT : Register Bits Masks Definitions */
#define TSI721_DEV_INT_INT_PC2SR                         ((uint32_t)0x00000004)
#define TSI721_DEV_INT_INT_I2C                           ((uint32_t)0x00000010)
#define TSI721_DEV_INT_INT_SRIO                          ((uint32_t)0x00000020)
#define TSI721_DEV_INT_INT_SR2PC_NONCH                   ((uint32_t)0x00000100)
#define TSI721_DEV_INT_INT_SR2PC_CH                      ((uint32_t)0x00000200)
#define TSI721_DEV_INT_INT_SMSG_NONCH                    ((uint32_t)0x00000400)
#define TSI721_DEV_INT_INT_SMSG_CH                       ((uint32_t)0x00000800)
#define TSI721_DEV_INT_INT_BDMA_NONCH                    ((uint32_t)0x00001000)
#define TSI721_DEV_INT_INT_BDMA_CH                       ((uint32_t)0x00002000)

/* TSI721_DEV_CHAN_INTE : Register Bits Masks Definitions */
#define TSI721_DEV_CHAN_INTE_INT_BDMA_CHAN_EN            ((uint32_t)0x000000ff)
#define TSI721_DEV_CHAN_INTE_INT_OBMSG_CHAN_EN           ((uint32_t)0x0000ff00)
#define TSI721_DEV_CHAN_INTE_INT_IBMSG_CHAN_EN           ((uint32_t)0x00ff0000)
#define TSI721_DEV_CHAN_INTE_INT_SR2PC_CHAN_EN           ((uint32_t)0xff000000)

/* TSI721_DEV_CHAN_INT : Register Bits Masks Definitions */
#define TSI721_DEV_CHAN_INT_INT_BDMA_CHAN                ((uint32_t)0x000000ff)
#define TSI721_DEV_CHAN_INT_INT_OBMSG_CHAN               ((uint32_t)0x0000ff00)
#define TSI721_DEV_CHAN_INT_INT_IBMSG_CHAN               ((uint32_t)0x00ff0000)
#define TSI721_DEV_CHAN_INT_INT_SR2PC_CHAN               ((uint32_t)0xff000000)

/* TSI721_INT_MOD : Register Bits Masks Definitions */
#define TSI721_INT_MOD_INT_MOD                           ((uint32_t)0xffffffff)

/* TSI721_RXPKT_SMSG_CNT : Register Bits Masks Definitions */
#define TSI721_RXPKT_SMSG_CNT_RXPKT_SMSG_CNT             ((uint32_t)0xffffffff)

/* TSI721_RXRSP_BDMA_CNT : Register Bits Masks Definitions */
#define TSI721_RXRSP_BDMA_CNT_RXRSP_BDMA_CNT             ((uint32_t)0xffffffff)

/* TSI721_RXPKT_BRG_CNT : Register Bits Masks Definitions */
#define TSI721_RXPKT_BRG_CNT_RXPKT_BRG_CNT               ((uint32_t)0xffffffff)

/* TSI721_TXTLP_SMSG_CNT : Register Bits Masks Definitions */
#define TSI721_TXTLP_SMSG_CNT_TXTLP_SMSG_CNT             ((uint32_t)0xffffffff)

/* TSI721_TXTLP_BDMA_CNT : Register Bits Masks Definitions */
#define TSI721_TXTLP_BDMA_CNT_TXTLP_BDMA_CNT             ((uint32_t)0xffffffff)

/* TSI721_TXTLP_BRG_CNT : Register Bits Masks Definitions */
#define TSI721_TXTLP_BRG_CNT_TXTLP_BRG_CNT               ((uint32_t)0xffffffff)

/* TSI721_BRG_PKT_ERR_CNT : Register Bits Masks Definitions */
#define TSI721_BRG_PKT_ERR_CNT_BRG_PKT_ERR_CNT           ((uint32_t)0xffffffff)

/* TSI721_MWR_CNT : Register Bits Masks Definitions */
#define TSI721_MWR_CNT_MW_OK_CNT                         ((uint32_t)0x0000ffff)
#define TSI721_MWR_CNT_MW_TOT_CNT                        ((uint32_t)0xffff0000)

/* TSI721_NWR_CNT : Register Bits Masks Definitions */
#define TSI721_NWR_CNT_NW_OK_CNT                         ((uint32_t)0x0000ffff)
#define TSI721_NWR_CNT_NW_TOT_CNT                        ((uint32_t)0xffff0000)

/* TSI721_MWR_LOG_DAT0 : Register Bits Masks Definitions */
#define TSI721_MWR_LOG_DAT0_DATA                         ((uint32_t)0xffffffff)

/* TSI721_MWR_LOG_DAT1 : Register Bits Masks Definitions */
#define TSI721_MWR_LOG_DAT1_DATA                         ((uint32_t)0xffffffff)

/* TSI721_MWR_LOG_DAT2 : Register Bits Masks Definitions */
#define TSI721_MWR_LOG_DAT2_DATA                         ((uint32_t)0xffffffff)

/* TSI721_MWR_LOG_DAT3 : Register Bits Masks Definitions */
#define TSI721_MWR_LOG_DAT3_DATA                         ((uint32_t)0xffffffff)

/* TSI721_NWR_LOG_DAT0 : Register Bits Masks Definitions */
#define TSI721_NWR_LOG_DAT0_DATA                         ((uint32_t)0xffffffff)

/* TSI721_NWR_LOG_DAT1 : Register Bits Masks Definitions */
#define TSI721_NWR_LOG_DAT1_DATA                         ((uint32_t)0xffffffff)

/* TSI721_NWR_LOG_DAT2 : Register Bits Masks Definitions */
#define TSI721_NWR_LOG_DAT2_DATA                         ((uint32_t)0xffffffff)

/* TSI721_NWR_LOG_DAT3 : Register Bits Masks Definitions */
#define TSI721_NWR_LOG_DAT3_DATA                         ((uint32_t)0xffffffff)

/* TSI721_MSIX_PBAL : Register Bits Masks Definitions */
#define TSI721_MSIX_PBAL_PENDING                         ((uint32_t)0xffffffff)

/* TSI721_MSIX_PBAM : Register Bits Masks Definitions */
#define TSI721_MSIX_PBAM_PENDING                         ((uint32_t)0xffffffff)

/* TSI721_MSIX_PBAU : Register Bits Masks Definitions */
#define TSI721_MSIX_PBAU_PENDING                         ((uint32_t)0x0000003f)

/* TSI721_MSIX_TAB_ADDRLX : Register Bits Masks Definitions */
#define TSI721_MSIX_TAB_ADDRLX_ADDR                      ((uint32_t)0xffffffff)

/* TSI721_MSIX_TAB_ADDRUX : Register Bits Masks Definitions */
#define TSI721_MSIX_TAB_ADDRUX_ADDR                      ((uint32_t)0xffffffff)

/* TSI721_MSIX_TAB_DATAX : Register Bits Masks Definitions */
#define TSI721_MSIX_TAB_DATAX_DATA                       ((uint32_t)0xffffffff)

/* TSI721_MSIX_TAB_MSKX : Register Bits Masks Definitions */
#define TSI721_MSIX_TAB_MSKX_MASK                        ((uint32_t)0x00000001)

/* TSI721_OBWINLBX : Register Bits Masks Definitions */
#define TSI721_OBWINLBX_WIN_EN                           ((uint32_t)0x00000001)
#define TSI721_OBWINLBX_ADD                              ((uint32_t)0xffff8000)

/* TSI721_OBWINUBX : Register Bits Masks Definitions */
#define TSI721_OBWINUBX_ADD                              ((uint32_t)0xffffffff)

/* TSI721_OBWINSZX : Register Bits Masks Definitions */
#define TSI721_OBWINSZX_SIZE                             ((uint32_t)0x00001f00)

/* TSI721_ZONE_SEL : Register Bits Masks Definitions */
#define TSI721_ZONE_SEL_ZONE_SEL                         ((uint32_t)0x00000007)
#define TSI721_ZONE_SEL_WIN_SEL                          ((uint32_t)0x00000038)
#define TSI721_ZONE_SEL_ZONE_GO                          ((uint32_t)0x00010000)
#define TSI721_ZONE_SEL_RD_WRB                           ((uint32_t)0x00020000)

/* TSI721_LUT_DATA0 : Register Bits Masks Definitions */
#define TSI721_LUT_DATA0_WR_TYPE                         ((uint32_t)0x0000000f)
#define TSI721_LUT_DATA0_WR_CRF                          ((uint32_t)0x00000010)
#define TSI721_LUT_DATA0_RD_CRF                          ((uint32_t)0x00000020)
#define TSI721_LUT_DATA0_RD_TYPE                         ((uint32_t)0x00000f00)
#define TSI721_LUT_DATA0_ADD                             ((uint32_t)0xfffff000)

/* TSI721_LUT_DATA1 : Register Bits Masks Definitions */
#define TSI721_LUT_DATA1_ADD                             ((uint32_t)0xffffffff)

/* TSI721_LUT_DATA2 : Register Bits Masks Definitions */
#define TSI721_LUT_DATA2_DEVICEID                        ((uint32_t)0x0000ffff)
#define TSI721_LUT_DATA2_TT                              ((uint32_t)0x00030000)
#define TSI721_LUT_DATA2_ADD                             ((uint32_t)0x000c0000)
#define TSI721_LUT_DATA2_HOP_CNT                         ((uint32_t)0xff000000)

/* TSI721_PC2SR_INTE : Register Bits Masks Definitions */
#define TSI721_PC2SR_INTE_ECC_CORR_EN                    ((uint32_t)0x00000002)
#define TSI721_PC2SR_INTE_ECC_UNCORR_EN                  ((uint32_t)0x00000004)

/* TSI721_PC2SR_INT : Register Bits Masks Definitions */
#define TSI721_PC2SR_INT_ECC_CORR                        ((uint32_t)0x00000002)
#define TSI721_PC2SR_INT_ECC_UNCORR                      ((uint32_t)0x00000004)

/* TSI721_PC2SR_INTSET : Register Bits Masks Definitions */
#define TSI721_PC2SR_INTSET_ECC_CORR_SET                 ((uint32_t)0x00000002)
#define TSI721_PC2SR_INTSET_ECC_UNCORR_SET               ((uint32_t)0x00000004)

/* TSI721_PC2SR_ECC_LOG : Register Bits Masks Definitions */
#define TSI721_PC2SR_ECC_LOG_ECC_CORR_MEM                ((uint32_t)0x00003fff)
#define TSI721_PC2SR_ECC_LOG_ECC_UNCORR_MEM              ((uint32_t)0x3fff0000)

/* TSI721_CPL_SMSG_CNT : Register Bits Masks Definitions */
#define TSI721_CPL_SMSG_CNT_CPL_SMSG_CNT                 ((uint32_t)0xffffffff)

/* TSI721_CPL_BDMA_CNT : Register Bits Masks Definitions */
#define TSI721_CPL_BDMA_CNT_CPL_BDMA_CNT                 ((uint32_t)0xffffffff)

/* TSI721_RXTLP_BRG_CNT : Register Bits Masks Definitions */
#define TSI721_RXTLP_BRG_CNT_RXTLP_BRG_CNT               ((uint32_t)0xffffffff)

/* TSI721_TXPKT_SMSG_CNT : Register Bits Masks Definitions */
#define TSI721_TXPKT_SMSG_CNT_TXPKT_SMSG_CNT             ((uint32_t)0xffffffff)

/* TSI721_TXPKT_BDMA_CNT : Register Bits Masks Definitions */
#define TSI721_TXPKT_BDMA_CNT_TXPKT_BDMA_CNT             ((uint32_t)0xffffffff)

/* TSI721_TXPKT_BRG_CNT : Register Bits Masks Definitions */
#define TSI721_TXPKT_BRG_CNT_TXPKT_BRG_CNT               ((uint32_t)0xffffffff)

/* TSI721_BRG_TLP_ERR_CNT : Register Bits Masks Definitions */
#define TSI721_BRG_TLP_ERR_CNT_BRG_TLP_ERR_CNT           ((uint32_t)0xffffffff)


/***********************************************************/
/* TSI721 : Messaging Register address offset definitions  */
/***********************************************************/

#define TSI721_RQRPTO                                    ((uint32_t)0x00060010)
#define TSI721_IB_DEVID                                  ((uint32_t)0x00060020)
#define TSI721_OBDMACXDWRCNT(X)              ((uint32_t)(0x61000 + 0x1000*(X)))
#define TSI721_OBDMACXDRDCNT(X)              ((uint32_t)(0x61004 + 0x1000*(X)))
#define TSI721_OBDMACXCTL(X)                 ((uint32_t)(0x61008 + 0x1000*(X)))
#define TSI721_OBDMACXINT(X)                 ((uint32_t)(0x6100c + 0x1000*(X)))
#define TSI721_OBDMACXINTSET(X)              ((uint32_t)(0x61010 + 0x1000*(X)))
#define TSI721_OBDMACXSTS(X)                 ((uint32_t)(0x61014 + 0x1000*(X)))
#define TSI721_OBDMACXINTE(X)                ((uint32_t)(0x61018 + 0x1000*(X)))
#define TSI721_OBDMACXPWE(X)                 ((uint32_t)(0x6101c + 0x1000*(X)))
#define TSI721_OBDMACXDPTRL(X)               ((uint32_t)(0x61020 + 0x1000*(X)))
#define TSI721_OBDMACXDPTRH(X)               ((uint32_t)(0x61024 + 0x1000*(X)))
#define TSI721_OBDMACXDSBL(X)                ((uint32_t)(0x61040 + 0x1000*(X)))
#define TSI721_OBDMACXDSBH(X)                ((uint32_t)(0x61044 + 0x1000*(X)))
#define TSI721_OBDMACXDSSZ(X)                ((uint32_t)(0x61048 + 0x1000*(X)))
#define TSI721_OBDMACXDSRP(X)                ((uint32_t)(0x6104c + 0x1000*(X)))
#define TSI721_OBDMACXDSWP(X)                ((uint32_t)(0x61050 + 0x1000*(X)))
#define TSI721_IBDMACXFQBL(X)                ((uint32_t)(0x61200 + 0x1000*(X)))
#define TSI721_IBDMACXFQBH(X)                ((uint32_t)(0x61204 + 0x1000*(X)))
#define TSI721_IBDMACXFQSZ(X)                ((uint32_t)(0x61208 + 0x1000*(X)))
#define TSI721_IBDMACXFQRP(X)                ((uint32_t)(0x6120c + 0x1000*(X)))
#define TSI721_IBDMACXFQWP(X)                ((uint32_t)(0x61210 + 0x1000*(X)))
#define TSI721_IBDMACXFQTH(X)                ((uint32_t)(0x61214 + 0x1000*(X)))
#define TSI721_IBDMACXCTL(X)                 ((uint32_t)(0x61240 + 0x1000*(X)))
#define TSI721_IBDMACXSTS(X)                 ((uint32_t)(0x61244 + 0x1000*(X)))
#define TSI721_IBDMACXINT(X)                 ((uint32_t)(0x61248 + 0x1000*(X)))
#define TSI721_IBDMACXINTSET(X)              ((uint32_t)(0x6124c + 0x1000*(X)))
#define TSI721_IBDMACXINTE(X)                ((uint32_t)(0x61250 + 0x1000*(X)))
#define TSI721_IBDMACXPWE(X)                 ((uint32_t)(0x61254 + 0x1000*(X)))
#define TSI721_IBDMACXDQBL(X)                ((uint32_t)(0x61300 + 0x1000*(X)))
#define TSI721_IBDMACXDQBH(X)                ((uint32_t)(0x61304 + 0x1000*(X)))
#define TSI721_IBDMACXDQRP(X)                ((uint32_t)(0x61308 + 0x1000*(X)))
#define TSI721_IBDMACXDQWP(X)                ((uint32_t)(0x6130c + 0x1000*(X)))
#define TSI721_IBDMACXDQSZ(X)                ((uint32_t)(0x61314 + 0x1000*(X)))
#define TSI721_SMSG_INTE                                 ((uint32_t)0x0006a000)
#define TSI721_SMSG_PWE                                  ((uint32_t)0x0006a004)
#define TSI721_SMSG_INT                                  ((uint32_t)0x0006a008)
#define TSI721_SMSG_PW                                   ((uint32_t)0x0006a00c)
#define TSI721_SMSG_INTSET                               ((uint32_t)0x0006a010)
#define TSI721_SMSG_ECC_LOG                              ((uint32_t)0x0006a014)
#define TSI721_RETRY_GEN_CNT                             ((uint32_t)0x0006a100)
#define TSI721_RETRY_RX_CNT                              ((uint32_t)0x0006a104)
#define TSI721_SMSG_ECC_CORRXLOG(X)           ((uint32_t)(0x6a300 + 0x004*(X)))
#define TSI721_SMSG_ECC_UNCORRXLOG(X)         ((uint32_t)(0x6a340 + 0x004*(X)))


/***********************************************************/
/* TSI721 : Messaging Register Bit Masks and Reset Values  */
/*           definitions for every register                */
/***********************************************************/

/* TSI721_RQRPTO : Register Bits Masks Definitions */
#define TSI721_RQRPTO_REQ_RSP_TO                         ((uint32_t)0x00ffffff)

/* TSI721_IB_DEVID : Register Bits Masks Definitions */
#define TSI721_IB_DEVID_DEVID                            ((uint32_t)0x0000ffff)

/* TSI721_OBDMACXDWRCNT : Register Bits Masks Definitions */
#define TSI721_OBDMACXDWRCNT_DWRCNT                      ((uint32_t)0xffffffff)

/* TSI721_OBDMACXDRDCNT : Register Bits Masks Definitions */
#define TSI721_OBDMACXDRDCNT_DRDCNT                      ((uint32_t)0xffffffff)

/* TSI721_OBDMACXCTL : Register Bits Masks Definitions */
#define TSI721_OBDMACXCTL_INIT                           ((uint32_t)0x00000001)
#define TSI721_OBDMACXCTL_SUSPEND                        ((uint32_t)0x00000002)
#define TSI721_OBDMACXCTL_RETRY_THR                      ((uint32_t)0x00000004)

/* TSI721_OBDMACXINT : Register Bits Masks Definitions */
#define TSI721_OBDMACXINT_IOF_DONE                       ((uint32_t)0x00000001)
#define TSI721_OBDMACXINT_ERROR                          ((uint32_t)0x00000002)
#define TSI721_OBDMACXINT_SUSPENDED                      ((uint32_t)0x00000004)
#define TSI721_OBDMACXINT_DONE                           ((uint32_t)0x00000008)
#define TSI721_OBDMACXINT_ST_FULL                        ((uint32_t)0x00000010)

/* TSI721_OBDMACXINTSET : Register Bits Masks Definitions */
#define TSI721_OBDMACXINTSET_IOF_DONE_SET                ((uint32_t)0x00000001)
#define TSI721_OBDMACXINTSET_ERROR_SET                   ((uint32_t)0x00000002)
#define TSI721_OBDMACXINTSET_SUSPENDED_SET               ((uint32_t)0x00000004)
#define TSI721_OBDMACXINTSET_DONE_SET                    ((uint32_t)0x00000008)
#define TSI721_OBDMACXINTSET_ST_FULL_SET                 ((uint32_t)0x00000010)

/* TSI721_OBDMACXSTS : Register Bits Masks Definitions */
#define TSI721_OBDMACXSTS_CS                             ((uint32_t)0x001f0000)
#define TSI721_OBDMACXSTS_RUN                            ((uint32_t)0x00200000)
#define TSI721_OBDMACXSTS_ABORT                          ((uint32_t)0x00400000)

/* TSI721_OBDMACXINTE : Register Bits Masks Definitions */
#define TSI721_OBDMACXINTE_IOF_DONE_EN                   ((uint32_t)0x00000001)
#define TSI721_OBDMACXINTE_ERROR_EN                      ((uint32_t)0x00000002)
#define TSI721_OBDMACXINTE_SUSPENDED_EN                  ((uint32_t)0x00000004)
#define TSI721_OBDMACXINTE_DONE_EN                       ((uint32_t)0x00000008)
#define TSI721_OBDMACXINTE_ST_FULL_EN                    ((uint32_t)0x00000010)

/* TSI721_OBDMACXPWE : Register Bits Masks Definitions */
#define TSI721_OBDMACXPWE_ERROR_EN                       ((uint32_t)0x00000002)

/* TSI721_OBDMACXDPTRL : Register Bits Masks Definitions */
#define TSI721_OBDMACXDPTRL_DPTRL                        ((uint32_t)0xfffffff0)

/* TSI721_OBDMACXDPTRH : Register Bits Masks Definitions */
#define TSI721_OBDMACXDPTRH_DPTRH                        ((uint32_t)0xffffffff)

/* TSI721_OBDMACXDSBL : Register Bits Masks Definitions */
#define TSI721_OBDMACXDSBL_ADD                           ((uint32_t)0xffffffc0)

/* TSI721_OBDMACXDSBH : Register Bits Masks Definitions */
#define TSI721_OBDMACXDSBH_ADD                           ((uint32_t)0xffffffff)

/* TSI721_OBDMACXDSSZ : Register Bits Masks Definitions */
#define TSI721_OBDMACXDSSZ_SIZE                          ((uint32_t)0x0000000f)

/* TSI721_OBDMACXDSRP : Register Bits Masks Definitions */
#define TSI721_OBDMACXDSRP_RD_PTR                        ((uint32_t)0x0007ffff)

/* TSI721_OBDMACXDSWP : Register Bits Masks Definitions */
#define TSI721_OBDMACXDSWP_WR_PTR                        ((uint32_t)0x0007ffff)

/* TSI721_IBDMACXFQBL : Register Bits Masks Definitions */
#define TSI721_IBDMACXFQBL_ADD                           ((uint32_t)0xffffffc0)

/* TSI721_IBDMACXFQBH : Register Bits Masks Definitions */
#define TSI721_IBDMACXFQBH_ADD                           ((uint32_t)0xffffffff)

/* TSI721_IBDMACXFQSZ : Register Bits Masks Definitions */
#define TSI721_IBDMACXFQSZ_SIZE                          ((uint32_t)0x0000000f)

/* TSI721_IBDMACXFQRP : Register Bits Masks Definitions */
#define TSI721_IBDMACXFQRP_RD_PTR                        ((uint32_t)0x0007ffff)

/* TSI721_IBDMACXFQWP : Register Bits Masks Definitions */
#define TSI721_IBDMACXFQWP_WR_PTR                        ((uint32_t)0x0007ffff)

/* TSI721_IBDMACXFQTH : Register Bits Masks Definitions */
#define TSI721_IBDMACXFQTH_IBFQ_TH                       ((uint32_t)0x0007ffff)

/* TSI721_IBDMACXCTL : Register Bits Masks Definitions */
#define TSI721_IBDMACXCTL_INIT                           ((uint32_t)0x00000001)
#define TSI721_IBDMACXCTL_SUSPEND                        ((uint32_t)0x00000002)

/* TSI721_IBDMACXSTS : Register Bits Masks Definitions */
#define TSI721_IBDMACXSTS_CS                             ((uint32_t)0x001f0000)
#define TSI721_IBDMACXSTS_RUN                            ((uint32_t)0x00200000)
#define TSI721_IBDMACXSTS_ABORT                          ((uint32_t)0x00400000)

/* TSI721_IBDMACXINT : Register Bits Masks Definitions */
#define TSI721_IBDMACXINT_DQ_RCV                         ((uint32_t)0x00000001)
#define TSI721_IBDMACXINT_FQ_LOW                         ((uint32_t)0x00000002)
#define TSI721_IBDMACXINT_PC_ERROR                       ((uint32_t)0x00000004)
#define TSI721_IBDMACXINT_SUSPENDED                      ((uint32_t)0x00000008)
#define TSI721_IBDMACXINT_SRTO                           ((uint32_t)0x00001000)
#define TSI721_IBDMACXINT_MASK ((uint32_t)(TSI721_IBDMACXINT_DQ_RCV | \
				TSI721_IBDMACXINT_FQ_LOW | \
				TSI721_IBDMACXINT_PC_ERROR | \
				TSI721_IBDMACXINT_SUSPENDED | \
				TSI721_IBDMACXINT_SRTO))

/* TSI721_IBDMACXINTSET : Register Bits Masks Definitions */
#define TSI721_IBDMACXINTSET_DQ_RCV_SET                  ((uint32_t)0x00000001)
#define TSI721_IBDMACXINTSET_FQ_LOW_SET                  ((uint32_t)0x00000002)
#define TSI721_IBDMACXINTSET_PC_ERROR_SET                ((uint32_t)0x00000004)
#define TSI721_IBDMACXINTSET_SUSPENDED_SET               ((uint32_t)0x00000008)
#define TSI721_IBDMACXINTSET_SRTO_SET                    ((uint32_t)0x00001000)

/* TSI721_IBDMACXINTE : Register Bits Masks Definitions */
#define TSI721_IBDMACXINTE_DQ_RCV_EN                     ((uint32_t)0x00000001)
#define TSI721_IBDMACXINTE_FQ_LOW_EN                     ((uint32_t)0x00000002)
#define TSI721_IBDMACXINTE_PC_ERROR_EN                   ((uint32_t)0x00000004)
#define TSI721_IBDMACXINTE_SUSPENDED_EN                  ((uint32_t)0x00000008)
#define TSI721_IBDMACXINTE_SRTO_EN                       ((uint32_t)0x00001000)

/* TSI721_IBDMACXPWE : Register Bits Masks Definitions */
#define TSI721_IBDMACXPWE_SRTO_EN                        ((uint32_t)0x00001000)

/* TSI721_IBDMACXDQBL : Register Bits Masks Definitions */
#define TSI721_IBDMACXDQBL_ADD                           ((uint32_t)0xffffffc0)

/* TSI721_IBDMACXDQBH : Register Bits Masks Definitions */
#define TSI721_IBDMACXDQBH_ADD                           ((uint32_t)0xffffffff)

/* TSI721_IBDMACXDQRP : Register Bits Masks Definitions */
#define TSI721_IBDMACXDQRP_RD_PTR                        ((uint32_t)0x0007ffff)

/* TSI721_IBDMACXDQWP : Register Bits Masks Definitions */
#define TSI721_IBDMACXDQWP_WR_PTR                        ((uint32_t)0x0007ffff)

/* TSI721_IBDMACXDQSZ : Register Bits Masks Definitions */
#define TSI721_IBDMACXDQSZ_SIZE                          ((uint32_t)0x0000000f)

/* TSI721_SMSG_INTE : Register Bits Masks Definitions */
#define TSI721_SMSG_INTE_ECC_CORR_CH_EN                  ((uint32_t)0x000000ff)
#define TSI721_SMSG_INTE_ECC_UNCORR_CH_EN                ((uint32_t)0x0000ff00)
#define TSI721_SMSG_INTE_ECC_CORR_EN                     ((uint32_t)0x00020000)
#define TSI721_SMSG_INTE_ECC_UNCORR_EN                   ((uint32_t)0x00040000)
#define TSI721_SMSG_INTE_UNS_RSP_EN                      ((uint32_t)0x00800000)

/* TSI721_SMSG_PWE : Register Bits Masks Definitions */
#define TSI721_SMSG_PWE_OBDMA_PW_EN                      ((uint32_t)0x000000ff)
#define TSI721_SMSG_PWE_IBDMA_PW_EN                      ((uint32_t)0x0000ff00)
#define TSI721_SMSG_PWE_UNS_RSP_EN                       ((uint32_t)0x00800000)

/* TSI721_SMSG_INT : Register Bits Masks Definitions */
#define TSI721_SMSG_INT_ECC_CORR_CH                      ((uint32_t)0x000000ff)
#define TSI721_SMSG_INT_ECC_UNCORR_CH                    ((uint32_t)0x0000ff00)
#define TSI721_SMSG_INT_ECC_CORR                         ((uint32_t)0x00020000)
#define TSI721_SMSG_INT_ECC_UNCORR                       ((uint32_t)0x00040000)
#define TSI721_SMSG_INT_UNS_RSP                          ((uint32_t)0x00800000)

/* TSI721_SMSG_PW : Register Bits Masks Definitions */
#define TSI721_SMSG_PW_OBDMA_PW                          ((uint32_t)0x000000ff)
#define TSI721_SMSG_PW_IBDMA_PW                          ((uint32_t)0x0000ff00)

/* TSI721_SMSG_INTSET : Register Bits Masks Definitions */
#define TSI721_SMSG_INTSET_ECC_CORR_CH_SET               ((uint32_t)0x000000ff)
#define TSI721_SMSG_INTSET_ECC_UNCORR_CH_SET             ((uint32_t)0x0000ff00)
#define TSI721_SMSG_INTSET_ECC_CORR_SET                  ((uint32_t)0x00020000)
#define TSI721_SMSG_INTSET_ECC_UNCORR_SET                ((uint32_t)0x00040000)
#define TSI721_SMSG_INTSET_UNS_RSP_SET                   ((uint32_t)0x00800000)

/* TSI721_SMSG_ECC_LOG : Register Bits Masks Definitions */
#define TSI721_SMSG_ECC_LOG_ECC_CORR_MEM                 ((uint32_t)0x00000007)
#define TSI721_SMSG_ECC_LOG_ECC_UNCORR_MEM               ((uint32_t)0x00070000)

/* TSI721_RETRY_GEN_CNT : Register Bits Masks Definitions */
#define TSI721_RETRY_GEN_CNT_RETRY_GEN_CNT               ((uint32_t)0xffffffff)

/* TSI721_RETRY_RX_CNT : Register Bits Masks Definitions */
#define TSI721_RETRY_RX_CNT_RETRY_RX_CNT                 ((uint32_t)0xffffffff)

/* TSI721_SMSG_ECC_CORRXLOG : Register Bits Masks Definitions */
#define TSI721_SMSG_ECC_CORRXLOG_ECC_CORR_MEM            ((uint32_t)0x000000ff)

/* TSI721_SMSG_ECC_UNCORRXLOG : Register Bits Masks Definitions */
#define TSI721_SMSG_ECC_UNCORRXLOG_ECC_UNCORR_MEM        ((uint32_t)0x000000ff)


/**********************************************************/
/* TSI721 : Block DMA Register address offset definitions */
/**********************************************************/

#define TSI721_DMACXDWRCNT(X)                ((uint32_t)(0x51000 + 0x1000*(X)))
#define TSI721_DMACXDRDCNT(X)                ((uint32_t)(0x51004 + 0x1000*(X)))
#define TSI721_DMACXCTL(X)                   ((uint32_t)(0x51008 + 0x1000*(X)))
#define TSI721_DMACXINT(X)                   ((uint32_t)(0x5100c + 0x1000*(X)))
#define TSI721_DMACXINTSET(X)                ((uint32_t)(0x51010 + 0x1000*(X)))
#define TSI721_DMACXSTS(X)                   ((uint32_t)(0x51014 + 0x1000*(X)))
#define TSI721_DMACXINTE(X)                  ((uint32_t)(0x51018 + 0x1000*(X)))
#define TSI721_DMACXDPTRL(X)                 ((uint32_t)(0x51024 + 0x1000*(X)))
#define TSI721_DMACXDPTRH(X)                 ((uint32_t)(0x51028 + 0x1000*(X)))
#define TSI721_DMACXDSBL(X)                  ((uint32_t)(0x5102c + 0x1000*(X)))
#define TSI721_DMACXDSBH(X)                  ((uint32_t)(0x51030 + 0x1000*(X)))
#define TSI721_DMACXDSSZ(X)                  ((uint32_t)(0x51034 + 0x1000*(X)))
#define TSI721_DMACXDSRP(X)                  ((uint32_t)(0x51038 + 0x1000*(X)))
#define TSI721_DMACXDSWP(X)                  ((uint32_t)(0x5103c + 0x1000*(X)))
#define TSI721_BDMA_INTE                                 ((uint32_t)0x0005f000)
#define TSI721_BDMA_INT                                  ((uint32_t)0x0005f004)
#define TSI721_BDMA_INTSET                               ((uint32_t)0x0005f008)
#define TSI721_BDMA_ECC_LOG                              ((uint32_t)0x0005f00c)
#define TSI721_BDMA_ECC_CORRXLOG(X)           ((uint32_t)(0x5f300 + 0x004*(X)))
#define TSI721_BDMA_ECC_UNCORRXLOG(X)         ((uint32_t)(0x5f340 + 0x004*(X)))


/**********************************************************/
/* TSI721 : Block DMA Register Bit Masks and Reset Values */
/*           definitions for every register               */
/**********************************************************/

/* TSI721_DMACXDWRCNT : Register Bits Masks Definitions */
#define TSI721_DMACXDWRCNT_DWRCNT                        ((uint32_t)0xffffffff)

/* TSI721_DMACXDRDCNT : Register Bits Masks Definitions */
#define TSI721_DMACXDRDCNT_DRDCNT                        ((uint32_t)0xffffffff)

/* TSI721_DMACXCTL : Register Bits Masks Definitions */
#define TSI721_DMACXCTL_INIT                             ((uint32_t)0x00000001)
#define TSI721_DMACXCTL_SUSPEND                          ((uint32_t)0x00000002)

/* TSI721_DMACXINT : Register Bits Masks Definitions */
#define TSI721_DMACXINT_IOF_DONE                         ((uint32_t)0x00000001)
#define TSI721_DMACXINT_ERROR                            ((uint32_t)0x00000002)
#define TSI721_DMACXINT_SUSPENDED                        ((uint32_t)0x00000004)
#define TSI721_DMACXINT_DONE                             ((uint32_t)0x00000008)
#define TSI721_DMACXINT_ST_FULL                          ((uint32_t)0x00000010)

/* TSI721_DMACXINTSET : Register Bits Masks Definitions */
#define TSI721_DMACXINTSET_IOF_DONE_SET                  ((uint32_t)0x00000001)
#define TSI721_DMACXINTSET_ERROR_SET                     ((uint32_t)0x00000002)
#define TSI721_DMACXINTSET_SUSPENDED_SET                 ((uint32_t)0x00000004)
#define TSI721_DMACXINTSET_DONE_SET                      ((uint32_t)0x00000008)
#define TSI721_DMACXINTSET_ST_FULL_SET                   ((uint32_t)0x00000010)

/* TSI721_DMACXSTS : Register Bits Masks Definitions */
#define TSI721_DMACXSTS_CS                               ((uint32_t)0x001f0000)
#define TSI721_DMACXSTS_RUN                              ((uint32_t)0x00200000)
#define TSI721_DMACXSTS_ABORT                            ((uint32_t)0x00400000)

/* TSI721_DMACXINTE : Register Bits Masks Definitions */
#define TSI721_DMACXINTE_IOF_DONE_EN                     ((uint32_t)0x00000001)
#define TSI721_DMACXINTE_ERROR_EN                        ((uint32_t)0x00000002)
#define TSI721_DMACXINTE_SUSPENDED_EN                    ((uint32_t)0x00000004)
#define TSI721_DMACXINTE_DONE_EN                         ((uint32_t)0x00000008)
#define TSI721_DMACXINTE_ST_FULL_EN                      ((uint32_t)0x00000010)

/* TSI721_DMACXDPTRL : Register Bits Masks Definitions */
#define TSI721_DMACXDPTRL_DPTRL                          ((uint32_t)0xffffffe0)

/* TSI721_DMACXDPTRH : Register Bits Masks Definitions */
#define TSI721_DMACXDPTRH_DPTRH                          ((uint32_t)0xffffffff)

/* TSI721_DMACXDSBL : Register Bits Masks Definitions */
#define TSI721_DMACXDSBL_ADD                             ((uint32_t)0xffffffc0)

/* TSI721_DMACXDSBH : Register Bits Masks Definitions */
#define TSI721_DMACXDSBH_ADD                             ((uint32_t)0xffffffff)

/* TSI721_DMACXDSSZ : Register Bits Masks Definitions */
#define TSI721_DMACXDSSZ_SIZE                            ((uint32_t)0x0000000f)

/* TSI721_DMACXDSRP : Register Bits Masks Definitions */
#define TSI721_DMACXDSRP_RD_PTR                          ((uint32_t)0x0007ffff)

/* TSI721_DMACXDSWP : Register Bits Masks Definitions */
#define TSI721_DMACXDSWP_WR_PTR                          ((uint32_t)0x0007ffff)

/* TSI721_BDMA_INTE : Register Bits Masks Definitions */
#define TSI721_BDMA_INTE_ECC_CORR_CH_EN                  ((uint32_t)0x000000ff)
#define TSI721_BDMA_INTE_ECC_UNCORR_CH_EN                ((uint32_t)0x0000ff00)
#define TSI721_BDMA_INTE_ECC_CORR_EN                     ((uint32_t)0x00020000)
#define TSI721_BDMA_INTE_ECC_UNCORR_EN                   ((uint32_t)0x00040000)

/* TSI721_BDMA_INT : Register Bits Masks Definitions */
#define TSI721_BDMA_INT_ECC_CORR_CH                      ((uint32_t)0x000000ff)
#define TSI721_BDMA_INT_ECC_UNCORR_CH                    ((uint32_t)0x0000ff00)
#define TSI721_BDMA_INT_ECC_CORR                         ((uint32_t)0x00020000)
#define TSI721_BDMA_INT_ECC_UNCORR                       ((uint32_t)0x00040000)

/* TSI721_BDMA_INTSET : Register Bits Masks Definitions */
#define TSI721_BDMA_INTSET_ECC_CORR_CH_SET               ((uint32_t)0x000000ff)
#define TSI721_BDMA_INTSET_ECC_UNCORR_CH_SET             ((uint32_t)0x0000ff00)
#define TSI721_BDMA_INTSET_ECC_CORR_SET                  ((uint32_t)0x00020000)
#define TSI721_BDMA_INTSET_ECC_UNCORR_SET                ((uint32_t)0x00040000)

/* TSI721_BDMA_ECC_LOG : Register Bits Masks Definitions */
#define TSI721_BDMA_ECC_LOG_ECC_CORR_MEM                 ((uint32_t)0x000000ff)
#define TSI721_BDMA_ECC_LOG_ECC_UNCORR_MEM               ((uint32_t)0x00ff0000)

/* TSI721_BDMA_ECC_CORRXLOG : Register Bits Masks Definitions */
#define TSI721_BDMA_ECC_CORRXLOG_ECC_CORR_MEM            ((uint32_t)0x0001ffff)

/* TSI721_BDMA_ECC_UNCORRXLOG : Register Bits Masks Definitions */
#define TSI721_BDMA_ECC_UNCORRXLOG_ECC_UNCORR_MEM        ((uint32_t)0x0001ffff)


/******************************************************/
/* TSI721 : I2C Register address offset definitions   */
/******************************************************/

#define TSI721_I2C_DEVID                                 ((uint32_t)0x00049100)
#define TSI721_I2C_RESET                                 ((uint32_t)0x00049104)
#define TSI721_I2C_MST_CFG                               ((uint32_t)0x00049108)
#define TSI721_I2C_MST_CNTRL                             ((uint32_t)0x0004910c)
#define TSI721_I2C_MST_RDATA                             ((uint32_t)0x00049110)
#define TSI721_I2C_MST_TDATA                             ((uint32_t)0x00049114)
#define TSI721_I2C_ACC_STAT                              ((uint32_t)0x00049118)
#define TSI721_I2C_INT_STAT                              ((uint32_t)0x0004911c)
#define TSI721_I2C_INT_ENABLE                            ((uint32_t)0x00049120)
#define TSI721_I2C_INT_SET                               ((uint32_t)0x00049124)
#define TSI721_I2C_SLV_CFG                               ((uint32_t)0x0004912c)
#define TSI721_I2C_BOOT_CNTRL                            ((uint32_t)0x00049140)
#define TSI721_EXI2C_REG_WADDR                           ((uint32_t)0x00049200)
#define TSI721_EXI2C_REG_WDATA                           ((uint32_t)0x00049204)
#define TSI721_EXI2C_REG_RADDR                           ((uint32_t)0x00049210)
#define TSI721_EXI2C_REG_RDATA                           ((uint32_t)0x00049214)
#define TSI721_EXI2C_ACC_STAT                            ((uint32_t)0x00049220)
#define TSI721_EXI2C_ACC_CNTRL                           ((uint32_t)0x00049224)
#define TSI721_EXI2C_STAT                                ((uint32_t)0x00049280)
#define TSI721_EXI2C_STAT_ENABLE                         ((uint32_t)0x00049284)
#define TSI721_EXI2C_MBOX_OUT                            ((uint32_t)0x00049290)
#define TSI721_EXI2C_MBOX_IN                             ((uint32_t)0x00049294)
#define TSI721_I2C_EVENT                                 ((uint32_t)0x00049300)
#define TSI721_I2C_SNAP_EVENT                            ((uint32_t)0x00049304)
#define TSI721_I2C_NEW_EVENT                             ((uint32_t)0x00049308)
#define TSI721_I2C_EVENT_ENB                             ((uint32_t)0x0004930c)
#define TSI721_I2C_DIVIDER                               ((uint32_t)0x00049320)
#define TSI721_I2C_START_SETUP_HOLD                      ((uint32_t)0x00049340)
#define TSI721_I2C_STOP_IDLE                             ((uint32_t)0x00049344)
#define TSI721_I2C_SDA_SETUP_HOLD                        ((uint32_t)0x00049348)
#define TSI721_I2C_SCL_PERIOD                            ((uint32_t)0x0004934c)
#define TSI721_I2C_SCL_MIN_PERIOD                        ((uint32_t)0x00049350)
#define TSI721_I2C_SCL_ARB_TIMEOUT                       ((uint32_t)0x00049354)
#define TSI721_I2C_BYTE_TRAN_TIMEOUT                     ((uint32_t)0x00049358)
#define TSI721_I2C_BOOT_DIAG_TIMER                       ((uint32_t)0x0004935c)
#define TSI721_I2C_BOOT_DIAG_PROGRESS                    ((uint32_t)0x000493b8)
#define TSI721_I2C_BOOT_DIAG_CFG                         ((uint32_t)0x000493bc)


/*****************************************************/
/* TSI721 : I2C Register Bit Masks and Reset Values  */
/*           definitions for every register          */
/*****************************************************/

/* TSI721_I2C_DEVID : Register Bits Masks Definitions */
#define TSI721_I2C_DEVID_REV                             ((uint32_t)0x0000000f)

/* TSI721_I2C_RESET : Register Bits Masks Definitions */
#define TSI721_I2C_RESET_SRESET                          ((uint32_t)0x80000000)

/* TSI721_I2C_MST_CFG : Register Bits Masks Definitions */
#define TSI721_I2C_MST_CFG_DEV_ADDR                      ((uint32_t)0x0000007f)
#define TSI721_I2C_MST_CFG_PA_SIZE                       ((uint32_t)0x00030000)
#define TSI721_I2C_MST_CFG_DORDER                        ((uint32_t)0x00800000)

/* TSI721_I2C_MST_CNTRL : Register Bits Masks Definitions */
#define TSI721_I2C_MST_CNTRL_PADDR                       ((uint32_t)0x0000ffff)
#define TSI721_I2C_MST_CNTRL_SIZE                        ((uint32_t)0x07000000)
#define TSI721_I2C_MST_CNTRL_WRITE                       ((uint32_t)0x40000000)
#define TSI721_I2C_MST_CNTRL_START                       ((uint32_t)0x80000000)

/* TSI721_I2C_MST_RDATA : Register Bits Masks Definitions */
#define TSI721_I2C_MST_RDATA_RBYTE0                      ((uint32_t)0x000000ff)
#define TSI721_I2C_MST_RDATA_RBYTE1                      ((uint32_t)0x0000ff00)
#define TSI721_I2C_MST_RDATA_RBYTE2                      ((uint32_t)0x00ff0000)
#define TSI721_I2C_MST_RDATA_RBYTE3                      ((uint32_t)0xff000000)

/* TSI721_I2C_MST_TDATA : Register Bits Masks Definitions */
#define TSI721_I2C_MST_TDATA_TBYTE0                      ((uint32_t)0x000000ff)
#define TSI721_I2C_MST_TDATA_TBYTE1                      ((uint32_t)0x0000ff00)
#define TSI721_I2C_MST_TDATA_TBYTE2                      ((uint32_t)0x00ff0000)
#define TSI721_I2C_MST_TDATA_TBYTE3                      ((uint32_t)0xff000000)

/* TSI721_I2C_ACC_STAT : Register Bits Masks Definitions */
#define TSI721_I2C_ACC_STAT_MST_NBYTES                   ((uint32_t)0x0000000f)
#define TSI721_I2C_ACC_STAT_MST_AN                       ((uint32_t)0x00000100)
#define TSI721_I2C_ACC_STAT_MST_PHASE                    ((uint32_t)0x00000e00)
#define TSI721_I2C_ACC_STAT_MST_ACTIVE                   ((uint32_t)0x00008000)
#define TSI721_I2C_ACC_STAT_SLV_PA                       ((uint32_t)0x00ff0000)
#define TSI721_I2C_ACC_STAT_SLV_AN                       ((uint32_t)0x01000000)
#define TSI721_I2C_ACC_STAT_SLV_PHASE                    ((uint32_t)0x06000000)
#define TSI721_I2C_ACC_STAT_SLV_WAIT                     ((uint32_t)0x08000000)
#define TSI721_I2C_ACC_STAT_BUS_ACTIVE                   ((uint32_t)0x40000000)
#define TSI721_I2C_ACC_STAT_SLV_ACTIVE                   ((uint32_t)0x80000000)

/* TSI721_I2C_INT_STAT : Register Bits Masks Definitions */
#define TSI721_I2C_INT_STAT_MA_OK                        ((uint32_t)0x00000001)
#define TSI721_I2C_INT_STAT_MA_ATMO                      ((uint32_t)0x00000002)
#define TSI721_I2C_INT_STAT_MA_NACK                      ((uint32_t)0x00000004)
#define TSI721_I2C_INT_STAT_MA_TMO                       ((uint32_t)0x00000008)
#define TSI721_I2C_INT_STAT_MA_COL                       ((uint32_t)0x00000010)
#define TSI721_I2C_INT_STAT_MA_DIAG                      ((uint32_t)0x00000080)
#define TSI721_I2C_INT_STAT_SA_OK                        ((uint32_t)0x00000100)
#define TSI721_I2C_INT_STAT_SA_READ                      ((uint32_t)0x00000200)
#define TSI721_I2C_INT_STAT_SA_WRITE                     ((uint32_t)0x00000400)
#define TSI721_I2C_INT_STAT_SA_FAIL                      ((uint32_t)0x00000800)
#define TSI721_I2C_INT_STAT_BL_OK                        ((uint32_t)0x00010000)
#define TSI721_I2C_INT_STAT_BL_FAIL                      ((uint32_t)0x00020000)
#define TSI721_I2C_INT_STAT_IMB_FULL                     ((uint32_t)0x01000000)
#define TSI721_I2C_INT_STAT_OMB_EMPTY                    ((uint32_t)0x02000000)

/* TSI721_I2C_INT_ENABLE : Register Bits Masks Definitions */
#define TSI721_I2C_INT_ENABLE_MA_OK                      ((uint32_t)0x00000001)
#define TSI721_I2C_INT_ENABLE_MA_ATMO                    ((uint32_t)0x00000002)
#define TSI721_I2C_INT_ENABLE_MA_NACK                    ((uint32_t)0x00000004)
#define TSI721_I2C_INT_ENABLE_MA_TMO                     ((uint32_t)0x00000008)
#define TSI721_I2C_INT_ENABLE_MA_COL                     ((uint32_t)0x00000010)
#define TSI721_I2C_INT_ENABLE_MA_DIAG                    ((uint32_t)0x00000080)
#define TSI721_I2C_INT_ENABLE_SA_OK                      ((uint32_t)0x00000100)
#define TSI721_I2C_INT_ENABLE_SA_READ                    ((uint32_t)0x00000200)
#define TSI721_I2C_INT_ENABLE_SA_WRITE                   ((uint32_t)0x00000400)
#define TSI721_I2C_INT_ENABLE_SA_FAIL                    ((uint32_t)0x00000800)
#define TSI721_I2C_INT_ENABLE_BL_OK                      ((uint32_t)0x00010000)
#define TSI721_I2C_INT_ENABLE_BL_FAIL                    ((uint32_t)0x00020000)
#define TSI721_I2C_INT_ENABLE_IMB_FULL                   ((uint32_t)0x01000000)
#define TSI721_I2C_INT_ENABLE_OMB_EMPTY                  ((uint32_t)0x02000000)

/* TSI721_I2C_INT_SET : Register Bits Masks Definitions */
#define TSI721_I2C_INT_SET_MA_OK                         ((uint32_t)0x00000001)
#define TSI721_I2C_INT_SET_MA_ATMO                       ((uint32_t)0x00000002)
#define TSI721_I2C_INT_SET_MA_NACK                       ((uint32_t)0x00000004)
#define TSI721_I2C_INT_SET_MA_TMO                        ((uint32_t)0x00000008)
#define TSI721_I2C_INT_SET_MA_COL                        ((uint32_t)0x00000010)
#define TSI721_I2C_INT_SET_MA_DIAG                       ((uint32_t)0x00000080)
#define TSI721_I2C_INT_SET_SA_OK                         ((uint32_t)0x00000100)
#define TSI721_I2C_INT_SET_SA_READ                       ((uint32_t)0x00000200)
#define TSI721_I2C_INT_SET_SA_WRITE                      ((uint32_t)0x00000400)
#define TSI721_I2C_INT_SET_SA_FAIL                       ((uint32_t)0x00000800)
#define TSI721_I2C_INT_SET_BL_OK                         ((uint32_t)0x00010000)
#define TSI721_I2C_INT_SET_BL_FAIL                       ((uint32_t)0x00020000)
#define TSI721_I2C_INT_SET_IMB_FULL                      ((uint32_t)0x01000000)
#define TSI721_I2C_INT_SET_OMB_EMPTY                     ((uint32_t)0x02000000)

/* TSI721_I2C_SLV_CFG : Register Bits Masks Definitions */
#define TSI721_I2C_SLV_CFG_SLV_ADDR                      ((uint32_t)0x0000007f)
#define TSI721_I2C_SLV_CFG_SLV_UNLK                      ((uint32_t)0x01000000)
#define TSI721_I2C_SLV_CFG_SLV_EN                        ((uint32_t)0x10000000)
#define TSI721_I2C_SLV_CFG_ALRT_EN                       ((uint32_t)0x20000000)
#define TSI721_I2C_SLV_CFG_WR_EN                         ((uint32_t)0x40000000)
#define TSI721_I2C_SLV_CFG_RD_EN                         ((uint32_t)0x80000000)

/* TSI721_I2C_BOOT_CNTRL : Register Bits Masks Definitions */
#define TSI721_I2C_BOOT_CNTRL_PADDR                      ((uint32_t)0x00001fff)
#define TSI721_I2C_BOOT_CNTRL_PAGE_MODE                  ((uint32_t)0x0000e000)
#define TSI721_I2C_BOOT_CNTRL_BOOT_ADDR                  ((uint32_t)0x007f0000)
#define TSI721_I2C_BOOT_CNTRL_BUNLK                      ((uint32_t)0x10000000)
#define TSI721_I2C_BOOT_CNTRL_BINC                       ((uint32_t)0x20000000)
#define TSI721_I2C_BOOT_CNTRL_PSIZE                      ((uint32_t)0x40000000)
#define TSI721_I2C_BOOT_CNTRL_CHAIN                      ((uint32_t)0x80000000)

/* TSI721_EXI2C_REG_WADDR : Register Bits Masks Definitions */
#define TSI721_EXI2C_REG_WADDR_ADDR                      ((uint32_t)0xfffffffc)

/* TSI721_EXI2C_REG_WDATA : Register Bits Masks Definitions */
#define TSI721_EXI2C_REG_WDATA_WDATA                     ((uint32_t)0xffffffff)

/* TSI721_EXI2C_REG_RADDR : Register Bits Masks Definitions */
#define TSI721_EXI2C_REG_RADDR_ADDR                      ((uint32_t)0xfffffffc)

/* TSI721_EXI2C_REG_RDATA : Register Bits Masks Definitions */
#define TSI721_EXI2C_REG_RDATA_RDATA                     ((uint32_t)0xffffffff)

/* TSI721_EXI2C_ACC_STAT : Register Bits Masks Definitions */
#define TSI721_EXI2C_ACC_STAT_ALERT_FLAG                 ((uint32_t)0x00000001)
#define TSI721_EXI2C_ACC_STAT_IMB_FLAG                   ((uint32_t)0x00000004)
#define TSI721_EXI2C_ACC_STAT_OMB_FLAG                   ((uint32_t)0x00000008)
#define TSI721_EXI2C_ACC_STAT_ACC_OK                     ((uint32_t)0x00000080)

/* TSI721_EXI2C_ACC_CNTRL : Register Bits Masks Definitions */
#define TSI721_EXI2C_ACC_CNTRL_WINC                      ((uint32_t)0x00000004)
#define TSI721_EXI2C_ACC_CNTRL_RINC                      ((uint32_t)0x00000008)
#define TSI721_EXI2C_ACC_CNTRL_WSIZE                     ((uint32_t)0x00000030)
#define TSI721_EXI2C_ACC_CNTRL_RSIZE                     ((uint32_t)0x000000c0)

/* TSI721_EXI2C_STAT : Register Bits Masks Definitions */
#define TSI721_EXI2C_STAT_BDMA_NONCH                     ((uint32_t)0x00000001)
#define TSI721_EXI2C_STAT_SR2PC_NONCH                    ((uint32_t)0x00000002)
#define TSI721_EXI2C_STAT_PC2SR                          ((uint32_t)0x00000004)
#define TSI721_EXI2C_STAT_SMSG_NONCH                     ((uint32_t)0x00000008)
#define TSI721_EXI2C_STAT_DL_DOWN                        ((uint32_t)0x00000010)
#define TSI721_EXI2C_STAT_SRIO_MAC                       ((uint32_t)0x00000020)
#define TSI721_EXI2C_STAT_ECC_UNCORR                     ((uint32_t)0x00000040)
#define TSI721_EXI2C_STAT_I2C                            ((uint32_t)0x02000000)
#define TSI721_EXI2C_STAT_IMBR                           ((uint32_t)0x04000000)
#define TSI721_EXI2C_STAT_OMBW                           ((uint32_t)0x08000000)
#define TSI721_EXI2C_STAT_SW_STAT0                       ((uint32_t)0x10000000)
#define TSI721_EXI2C_STAT_SW_STAT1                       ((uint32_t)0x20000000)
#define TSI721_EXI2C_STAT_SW_STAT2                       ((uint32_t)0x40000000)
#define TSI721_EXI2C_STAT_RESET                          ((uint32_t)0x80000000)

/* TSI721_EXI2C_STAT_ENABLE : Register Bits Masks Definitions */
#define TSI721_EXI2C_STAT_ENABLE_BDMA_NONCH              ((uint32_t)0x00000001)
#define TSI721_EXI2C_STAT_ENABLE_SR2PC_NONCH             ((uint32_t)0x00000002)
#define TSI721_EXI2C_STAT_ENABLE_PC2SR                   ((uint32_t)0x00000004)
#define TSI721_EXI2C_STAT_ENABLE_SMSG_NONCH              ((uint32_t)0x00000008)
#define TSI721_EXI2C_STAT_ENABLE_DL_DOWN                 ((uint32_t)0x00000010)
#define TSI721_EXI2C_STAT_ENABLE_SRIO_MAC                ((uint32_t)0x00000020)
#define TSI721_EXI2C_STAT_ENABLE_ECC_UNCORR              ((uint32_t)0x00000040)
#define TSI721_EXI2C_STAT_ENABLE_I2C                     ((uint32_t)0x02000000)
#define TSI721_EXI2C_STAT_ENABLE_IMBR                    ((uint32_t)0x04000000)
#define TSI721_EXI2C_STAT_ENABLE_OMBW                    ((uint32_t)0x08000000)
#define TSI721_EXI2C_STAT_ENABLE_SW_STAT0                ((uint32_t)0x10000000)
#define TSI721_EXI2C_STAT_ENABLE_SW_STAT1                ((uint32_t)0x20000000)
#define TSI721_EXI2C_STAT_ENABLE_SW_STAT2                ((uint32_t)0x40000000)
#define TSI721_EXI2C_STAT_ENABLE_RESET                   ((uint32_t)0x80000000)

/* TSI721_EXI2C_MBOX_OUT : Register Bits Masks Definitions */
#define TSI721_EXI2C_MBOX_OUT_DATA                       ((uint32_t)0xffffffff)

/* TSI721_EXI2C_MBOX_IN : Register Bits Masks Definitions */
#define TSI721_EXI2C_MBOX_IN_DATA                        ((uint32_t)0xffffffff)

/* TSI721_I2C_X : Register Bits Masks Definitions */
#define TSI721_I2C_X_MARBTO                              ((uint32_t)0x00000001)
#define TSI721_I2C_X_MSCLTO                              ((uint32_t)0x00000002)
#define TSI721_I2C_X_MBTTO                               ((uint32_t)0x00000004)
#define TSI721_I2C_X_MTRTO                               ((uint32_t)0x00000008)
#define TSI721_I2C_X_MCOL                                ((uint32_t)0x00000010)
#define TSI721_I2C_X_MNACK                               ((uint32_t)0x00000020)
#define TSI721_I2C_X_BLOK                                ((uint32_t)0x00000100)
#define TSI721_I2C_X_BLNOD                               ((uint32_t)0x00000200)
#define TSI721_I2C_X_BLSZ                                ((uint32_t)0x00000400)
#define TSI721_I2C_X_BLERR                               ((uint32_t)0x00000800)
#define TSI721_I2C_X_BLTO                                ((uint32_t)0x00001000)
#define TSI721_I2C_X_MTD                                 ((uint32_t)0x00004000)
#define TSI721_I2C_X_SSCLTO                              ((uint32_t)0x00020000)
#define TSI721_I2C_X_SBTTO                               ((uint32_t)0x00040000)
#define TSI721_I2C_X_STRTO                               ((uint32_t)0x00080000)
#define TSI721_I2C_X_SCOL                                ((uint32_t)0x00100000)
#define TSI721_I2C_X_OMBR                                ((uint32_t)0x00400000)
#define TSI721_I2C_X_IMBW                                ((uint32_t)0x00800000)
#define TSI721_I2C_X_DCMDD                               ((uint32_t)0x01000000)
#define TSI721_I2C_X_DHIST                               ((uint32_t)0x02000000)
#define TSI721_I2C_X_DTIMER                              ((uint32_t)0x04000000)
#define TSI721_I2C_X_SD                                  ((uint32_t)0x10000000)
#define TSI721_I2C_X_SDR                                 ((uint32_t)0x20000000)
#define TSI721_I2C_X_SDW                                 ((uint32_t)0x40000000)

/* TSI721_I2C_NEW_EVENT : Register Bits Masks Definitions */
#define TSI721_I2C_NEW_EVENT_MARBTO                      ((uint32_t)0x00000001)
#define TSI721_I2C_NEW_EVENT_MSCLTO                      ((uint32_t)0x00000002)
#define TSI721_I2C_NEW_EVENT_MBTTO                       ((uint32_t)0x00000004)
#define TSI721_I2C_NEW_EVENT_MTRTO                       ((uint32_t)0x00000008)
#define TSI721_I2C_NEW_EVENT_MCOL                        ((uint32_t)0x00000010)
#define TSI721_I2C_NEW_EVENT_MNACK                       ((uint32_t)0x00000020)
#define TSI721_I2C_NEW_EVENT_BLOK                        ((uint32_t)0x00000100)
#define TSI721_I2C_NEW_EVENT_BLNOD                       ((uint32_t)0x00000200)
#define TSI721_I2C_NEW_EVENT_BLSZ                        ((uint32_t)0x00000400)
#define TSI721_I2C_NEW_EVENT_BLERR                       ((uint32_t)0x00000800)
#define TSI721_I2C_NEW_EVENT_BLTO                        ((uint32_t)0x00001000)
#define TSI721_I2C_NEW_EVENT_MTD                         ((uint32_t)0x00004000)
#define TSI721_I2C_NEW_EVENT_SSCLTO                      ((uint32_t)0x00020000)
#define TSI721_I2C_NEW_EVENT_SBTTO                       ((uint32_t)0x00040000)
#define TSI721_I2C_NEW_EVENT_STRTO                       ((uint32_t)0x00080000)
#define TSI721_I2C_NEW_EVENT_SCOL                        ((uint32_t)0x00100000)
#define TSI721_I2C_NEW_EVENT_OMBR                        ((uint32_t)0x00400000)
#define TSI721_I2C_NEW_EVENT_IMBW                        ((uint32_t)0x00800000)
#define TSI721_I2C_NEW_EVENT_DCMDD                       ((uint32_t)0x01000000)
#define TSI721_I2C_NEW_EVENT_DHIST                       ((uint32_t)0x02000000)
#define TSI721_I2C_NEW_EVENT_DTIMER                      ((uint32_t)0x04000000)
#define TSI721_I2C_NEW_EVENT_SD                          ((uint32_t)0x10000000)
#define TSI721_I2C_NEW_EVENT_SDR                         ((uint32_t)0x20000000)
#define TSI721_I2C_NEW_EVENT_SDW                         ((uint32_t)0x40000000)

/* TSI721_I2C_EVENT_ENB : Register Bits Masks Definitions */
#define TSI721_I2C_EVENT_ENB_MARBTO                      ((uint32_t)0x00000001)
#define TSI721_I2C_EVENT_ENB_MSCLTO                      ((uint32_t)0x00000002)
#define TSI721_I2C_EVENT_ENB_MBTTO                       ((uint32_t)0x00000004)
#define TSI721_I2C_EVENT_ENB_MTRTO                       ((uint32_t)0x00000008)
#define TSI721_I2C_EVENT_ENB_MCOL                        ((uint32_t)0x00000010)
#define TSI721_I2C_EVENT_ENB_MNACK                       ((uint32_t)0x00000020)
#define TSI721_I2C_EVENT_ENB_BLOK                        ((uint32_t)0x00000100)
#define TSI721_I2C_EVENT_ENB_BLNOD                       ((uint32_t)0x00000200)
#define TSI721_I2C_EVENT_ENB_BLSZ                        ((uint32_t)0x00000400)
#define TSI721_I2C_EVENT_ENB_BLERR                       ((uint32_t)0x00000800)
#define TSI721_I2C_EVENT_ENB_BLTO                        ((uint32_t)0x00001000)
#define TSI721_I2C_EVENT_ENB_MTD                         ((uint32_t)0x00004000)
#define TSI721_I2C_EVENT_ENB_SSCLTO                      ((uint32_t)0x00020000)
#define TSI721_I2C_EVENT_ENB_SBTTO                       ((uint32_t)0x00040000)
#define TSI721_I2C_EVENT_ENB_STRTO                       ((uint32_t)0x00080000)
#define TSI721_I2C_EVENT_ENB_SCOL                        ((uint32_t)0x00100000)
#define TSI721_I2C_EVENT_ENB_OMBR                        ((uint32_t)0x00400000)
#define TSI721_I2C_EVENT_ENB_IMBW                        ((uint32_t)0x00800000)
#define TSI721_I2C_EVENT_ENB_DCMDD                       ((uint32_t)0x01000000)
#define TSI721_I2C_EVENT_ENB_DHIST                       ((uint32_t)0x02000000)
#define TSI721_I2C_EVENT_ENB_DTIMER                      ((uint32_t)0x04000000)
#define TSI721_I2C_EVENT_ENB_SD                          ((uint32_t)0x10000000)
#define TSI721_I2C_EVENT_ENB_SDR                         ((uint32_t)0x20000000)
#define TSI721_I2C_EVENT_ENB_SDW                         ((uint32_t)0x40000000)

/* TSI721_I2C_DIVIDER : Register Bits Masks Definitions */
#define TSI721_I2C_DIVIDER_MSDIV                         ((uint32_t)0x00000fff)
#define TSI721_I2C_DIVIDER_USDIV                         ((uint32_t)0x0fff0000)

/* TSI721_I2C_START_SETUP_HOLD : Register Bits Masks Definitions */
#define TSI721_I2C_START_SETUP_HOLD_START_HOLD           ((uint32_t)0x0000ffff)
#define TSI721_I2C_START_SETUP_HOLD_START_SETUP          ((uint32_t)0xffff0000)

/* TSI721_I2C_STOP_IDLE : Register Bits Masks Definitions */
#define TSI721_I2C_STOP_IDLE_IDLE_DET                    ((uint32_t)0x0000ffff)
#define TSI721_I2C_STOP_IDLE_STOP_SETUP                  ((uint32_t)0xffff0000)

/* TSI721_I2C_SDA_SETUP_HOLD : Register Bits Masks Definitions */
#define TSI721_I2C_SDA_SETUP_HOLD_SDA_HOLD               ((uint32_t)0x0000ffff)
#define TSI721_I2C_SDA_SETUP_HOLD_SDA_SETUP              ((uint32_t)0xffff0000)

/* TSI721_I2C_SCL_PERIOD : Register Bits Masks Definitions */
#define TSI721_I2C_SCL_PERIOD_SCL_LOW                    ((uint32_t)0x0000ffff)
#define TSI721_I2C_SCL_PERIOD_SCL_HIGH                   ((uint32_t)0xffff0000)

/* TSI721_I2C_SCL_MIN_PERIOD : Register Bits Masks Definitions */
#define TSI721_I2C_SCL_MIN_PERIOD_SCL_MINL               ((uint32_t)0x0000ffff)
#define TSI721_I2C_SCL_MIN_PERIOD_SCL_MINH               ((uint32_t)0xffff0000)

/* TSI721_I2C_SCL_ARB_TIMEOUT : Register Bits Masks Definitions */
#define TSI721_I2C_SCL_ARB_TIMEOUT_ARB_TO                ((uint32_t)0x0000ffff)
#define TSI721_I2C_SCL_ARB_TIMEOUT_SCL_TO                ((uint32_t)0xffff0000)

/* TSI721_I2C_BYTE_TRAN_TIMEOUT : Register Bits Masks Definitions */
#define TSI721_I2C_BYTE_TRAN_TIMEOUT_TRAN_TO             ((uint32_t)0x0000ffff)
#define TSI721_I2C_BYTE_TRAN_TIMEOUT_BYTE_TO             ((uint32_t)0xffff0000)

/* TSI721_I2C_BOOT_DIAG_TIMER : Register Bits Masks Definitions */
#define TSI721_I2C_BOOT_DIAG_TIMER_COUNT                 ((uint32_t)0x0000ffff)
#define TSI721_I2C_BOOT_DIAG_TIMER_FREERUN               ((uint32_t)0x80000000)

/* TSI721_I2C_BOOT_DIAG_PROGRESS : Register Bits Masks Definitions */
#define TSI721_I2C_BOOT_DIAG_PROGRESS_PADDR              ((uint32_t)0x0000ffff)
#define TSI721_I2C_BOOT_DIAG_PROGRESS_REGCNT             ((uint32_t)0xffff0000)

/* TSI721_I2C_BOOT_DIAG_CFG : Register Bits Masks Definitions */
#define TSI721_I2C_BOOT_DIAG_CFG_BOOT_ADDR               ((uint32_t)0x0000007f)
#define TSI721_I2C_BOOT_DIAG_CFG_PINC                    ((uint32_t)0x10000000)
#define TSI721_I2C_BOOT_DIAG_CFG_PASIZE                  ((uint32_t)0x20000000)
#define TSI721_I2C_BOOT_DIAG_CFG_BDIS                    ((uint32_t)0x40000000)
#define TSI721_I2C_BOOT_DIAG_CFG_BOOTING                 ((uint32_t)0x80000000)


/********************************************************/
/* TSI721 : GPIO Register address offset definitions    */
/********************************************************/

#define TSI721_GPIO0_DATA                                ((uint32_t)0x0004a000)
#define TSI721_GPIO0_CNTRL                               ((uint32_t)0x0004a004)

/********************************************************/
/* TSI721 : GPIO Register Bit Masks and Reset Values    */
/*           definitions for every register             */
/********************************************************/

/* TSI721_GPIO0_DATA : Register Bits Masks Definitions */
#define TSI721_GPIO0_DATA_GPIO_DATA_OUT                  ((uint32_t)0x0000ffff)
#define TSI721_GPIO0_DATA_GPIO_DATA_IN                   ((uint32_t)0xffff0000)

/* TSI721_GPIO0_CNTRL : Register Bits Masks Definitions */
#define TSI721_GPIO0_CNTRL_GPIO_CFG                      ((uint32_t)0x0000ffff)
#define TSI721_GPIO0_CNTRL_GPIO_DIR                      ((uint32_t)0xffff0000)


/*******************************************************/
/* TSI721 : SerDes Register address offset definitions */
/*******************************************************/

#define TSI721_PCIE_SERDES_BASE                          ((uint32_t)0x0004c000)
#define TSI721_SRIO_SERDES_BASE                          ((uint32_t)0x0004e000)

/* SerDes register offsets are from base offsets shown above */

#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN(X) ((uint32_t)(0x1000 + 0x400*(X)))
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN(X) ((uint32_t)(0x100c + 0x400*(X)))

#define TSI721_SRIO_SERDES_LANEX_DIG_TX_OVRD_IN(x)  ((uint32_t)(TSI721_SRIO_SERDES_BASE + \
		                                     TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN(x)))
#define TSI721_SRIO_SERDES_LANEX_DIG_RX_OVRD_IN(x)  ((uint32_t)(TSI721_SRIO_SERDES_BASE + \
		                                     TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN(x)))

/*******************************************************/
/* TSI721 : SerDes Register Bit Masks and Reset Values */
/*           definitions for every register            */
/*******************************************************/

/* TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN : Register Bits Masks Definitions */

#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_HALF_RATE ((uint32_t)0x00000001)
#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_LOOPBK_EN ((uint32_t)0x00000002)
#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_INVERT  ((uint32_t)0x00000004)
#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_DATA_EN ((uint32_t)0x00000010)
#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_TX_EN   ((uint32_t)0x00000020)
#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_CM_EN   ((uint32_t)0x00000040)
#define TSI721_SERDES_LANEX_LANEN_DIG_TX_OVRD_IN_EN_L    ((uint32_t)0x00000200)

/* TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN : Register Bits Masks Definitions */
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN_INVERT  ((uint32_t)0x00000001)
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN_PLL_EN  ((uint32_t)0x00000004)
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN_DATA_EN ((uint32_t)0x00000008)
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN_ALIGN_EN ((uint32_t)0x00000010)
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN_TERM_EN ((uint32_t)0x00000040)
#define TSI721_SERDES_LANEX_LANEN_DIG_RX_OVRD_IN_EN      ((uint32_t)0x00000400)


/**********************************************************/
/* TSI721 : Top-level Register address offset definitions */
/**********************************************************/

#define TSI721_DEVSTAT                                  ((uint32_t)0x00048000)
#define TSI721_DEVCTL                                   ((uint32_t)0x00048004)
#define TSI721_CLK_GATE                                 ((uint32_t)0x00048008)
#define TSI721_JTAG_ID                                  ((uint32_t)0x0004800c)
#define TSI721_PC_TX_CTL                                ((uint32_t)0x00048200)
#define TSI721_PC_TX_CTL_2                              ((uint32_t)0x00048220)
#define TSI721_SR_TX_CTLX(X)                 ((uint32_t)(0x48800 + 0x020*(X)))

/**********************************************************/
/* TSI721 : Top-level Register Bit Masks and Reset Values */
/*           definitions for every register               */
/**********************************************************/

/* TSI721_DEVSTAT : Register Bits Masks Definitions */
#define TSI721_DEVSTAT_PCRDY                             ((uint32_t)0x00000002)
#define TSI721_DEVSTAT_STRAP_RATE                        ((uint32_t)0x00000e00)
#define TSI721_DEVSTAT_CLKSEL                            ((uint32_t)0x00003000)
#define TSI721_DEVSTAT_I2C_DISABLE                       ((uint32_t)0x00010000)
#define TSI721_DEVSTAT_I2C_SA                            ((uint32_t)0x001e0000)
#define TSI721_DEVSTAT_I2C_SEL                           ((uint32_t)0x01000000)
#define TSI721_DEVSTAT_SR_BOOT                           ((uint32_t)0x02000000)
#define TSI721_DEVSTAT_I2C_MA                            ((uint32_t)0x04000000)
#define TSI721_DEVSTAT_CLKMOD                            ((uint32_t)0x08000000)
#define TSI721_DEVSTAT_SP_SWAP_RX                        ((uint32_t)0x10000000)
#define TSI721_DEVSTAT_SP_SWAP_TX                        ((uint32_t)0x20000000)
#define TSI721_DEVSTAT_SP_HOST                           ((uint32_t)0x40000000)
#define TSI721_DEVSTAT_SP_DEVID                          ((uint32_t)0x80000000)

/* TSI721_DEVCTL : Register Bits Masks Definitions */
#define TSI721_DEVCTL_FRST                               ((uint32_t)0x00000001)
#define TSI721_DEVCTL_PCBOOT_CMPL                        ((uint32_t)0x00000002)
#define TSI721_DEVCTL_SRBOOT_CMPL                        ((uint32_t)0x00000004)
#define TSI721_DEVCTL_MECS_O                             ((uint32_t)0x00000010)
#define TSI721_DEVCTL_SR_RST_MODE                        ((uint32_t)0x000f0000)

#define TSI721_DEVCTL_SR_RST_MODE_HOT_RST                ((uint32_t)0x00000000)
#define TSI721_DEVCTL_SR_RST_MODE_SRIO_ONLY              ((uint32_t)0x00010000)

/* TSI721_CLK_GATE : Register Bits Masks Definitions */
#define TSI721_CLK_GATE_CLK_GATE_BDMACH                  ((uint32_t)0x000000ff)
#define TSI721_CLK_GATE_CLK_GATE_SMSGCH                  ((uint32_t)0x0000ff00)

/* TSI721_JTAG_ID : Register Bits Masks Definitions */
#define TSI721_JTAG_ID_MANU_ID                           ((uint32_t)0x00000ffe)
#define TSI721_JTAG_ID_PART                              ((uint32_t)0x0ffff000)
#define TSI721_JTAG_ID_VERSION                           ((uint32_t)0xf0000000)

/* TSI721_PC_TX_CTL : Register Bits Masks Definitions */
#define TSI721_PC_TX_CTL_TX_AMP_FULL                     ((uint32_t)0x0000003f)
#define TSI721_PC_TX_CTL_TX_COEF60_G2                    ((uint32_t)0x00001f00)
#define TSI721_PC_TX_CTL_TX_COEF35_G2                    ((uint32_t)0x001f0000)
/* TSI721_PC_TX_CTL_2 : Register Bits Masks Definitions */
#define TSI721_PC_TX_CTL_2_TX_AMP_LOW                    ((uint32_t)0x0000003f)
#define TSI721_PC_TX_CTL_2_TX_COEF35_G1                  ((uint32_t)0x001f0000)

/* TSI721_SR_TX_CTLX : Register Bits Masks Definitions */
#define TSI721_SR_TX_CTLX_TX_AMP                         ((uint32_t)0x0000003f)
#define TSI721_SR_TX_CTLX_TX_COEF                        ((uint32_t)0x00001f00)
#define TSI721_SR_TX_CTLX_LB_EN                          ((uint32_t)0x80000000)

/**********************************************************/
/* TSI721 : 5G Training Work Around Register definitions  */
/**********************************************************/

#define TSI721_5G_WA_REG0(X)                 ((uint32_t)(0x0004F054+(0x400*X)))
#define TSI721_5G_WA_REG1(X)                 ((uint32_t)(0x0004F04C+(0x400*X)))
#define TSI721_NUM_WA_REGS                                                    4
#define TSI721_WA_VAL_5G                                 ((uint32_t)0x0000006F)
#define TSI721_WA_VAL_3G                                 ((uint32_t)0x0000000F)

/******************************************************/
/* TSI721 : MESSAGE DESCRIPTOR BIT DEFINITIONS        */
/******************************************************/

#ifndef RIO_MAX_MSG_SIZE
#define RIO_MAX_MSG_SIZE                                     ((uint32_t)0x1000)
#endif
#define TSI721_MSG_BUFFER_SIZE                                 RIO_MAX_MSG_SIZE
#define TSI721_MSG_MAX_SIZE                                    RIO_MAX_MSG_SIZE
#define TSI721_IMSG_MAXCH                                         ((uint32_t)8)
#define TSI721_IMSGD_MIN_RING_SIZE                               ((uint32_t)32)

#define TSI721_OMSGD_MIN_RING_SIZE                               ((uint32_t)32)

/******************************************************/
/* Inbound Message Descriptors                        */
/******************************************************/

#define TSI721_IMD_DEVID                                 ((uint32_t)0x0000ffff)
#define TSI721_IMD_CRF                                   ((uint32_t)0x00010000)
#define TSI721_IMD_PRIO                                  ((uint32_t)0x00060000)
#define TSI721_IMD_TT                                    ((uint32_t)0x00180000)
#define TSI721_IMD_DTYPE                                 ((uint32_t)0xe0000000)
#define TSI721_IMD_BCOUNT                                ((uint32_t)0x00000ff8)
#define TSI721_IMD_SSIZE                                 ((uint32_t)0x0000f000)
#define TSI721_IMD_LETER                                 ((uint32_t)0x00030000)
#define TSI721_IMD_XMBOX                                 ((uint32_t)0x003c0000)
#define TSI721_IMD_MBOX                                  ((uint32_t)0x00c00000)
#define TSI721_IMD_CS                                    ((uint32_t)0x78000000)
#define TSI721_IMD_HO                                    ((uint32_t)0x80000000)

/******************************************************/
/* Outbound Message Descriptors                       */
/******************************************************/

#define TSI721_OMD_DEVID                                 ((uint32_t)0x0000ffff)
#define TSI721_OMD_CRF                                   ((uint32_t)0x00010000)
#define TSI721_OMD_PRIO                                  ((uint32_t)0x00060000)
#define TSI721_OMD_IOF                                   ((uint32_t)0x08000000)
#define TSI721_OMD_DTYPE                                 ((uint32_t)0xe0000000)
#define TSI721_OMD_RSRVD                                 ((uint32_t)0x17f80000)

#define TSI721_OMD_BCOUNT                                ((uint32_t)0x00000ff8)
#define TSI721_OMD_SSIZE                                 ((uint32_t)0x0000f000)
#define TSI721_OMD_LETTER                                ((uint32_t)0x00030000)
#define TSI721_OMD_XMBOX                                 ((uint32_t)0x003c0000)
#define TSI721_OMD_MBOX                                  ((uint32_t)0x00c00000)
#define TSI721_OMD_TT                                    ((uint32_t)0x0c000000)

#ifdef __cplusplus
}
#endif

#endif /* __TSI721_H__ */
