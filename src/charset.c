/*
 * Copyright (c) 1993-2025 Paul Mattes.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the names of Paul Mattes, Jeff Sparkes, GTRC nor the names of
 *       their contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL PAUL MATTES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * x029 character set definitions.
 */

#include <string.h>

#include "jones.h"
#include "charset.h"

struct charset {
    const char *name;
    const char *description;
    unsigned char punch_type; /* for image save */
    unsigned charset[256];
};

static struct charset charsets[] = {
    /* 029 keypunch. */
    { "029", "029 standard", PTYPE_029,
      {    NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000, 03002, 00006, 00102, 02102, 01042, 04000, 00022, /*  !"#$%&' */
        04022, 02022, 02042, 04012, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001, 00202, 02012, 04042, 00012, 01012, 01006, /* 89:;<=>? */
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001,    NS,    NS,    NS,    NS, 01022, /* XYZ[\]^_ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* `abcdefg */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* hijklmno */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* pqrstuvw */
           NS,    NS,    NS,    NS, 04006,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS, 05000,    NS,    NS,    NS,    NS,    NS, /*  ¡¢£¤¥¦§ */
           NS,    NS,    NS,    NS, 02006,    NS,    NS,    NS, /* ¨©ª«¬­®¯ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* °±²³´µ¶· */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¸¹º»¼½¾¿ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÀÁÂÃÄÅÆÇ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÈÉÊËÌÍÎÏ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÐÑÒÓÔÕÖ× */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ØÙÚÛÜÝÞß */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* àáâãäåæç */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* èéêëìíîï */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ðñòóôõö÷ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS  /* øùúûüýþÿ */
      }
    },
    /* 026 keypunch FORTRAN. */
    { "bcd-h", "026 FORTRAN", PTYPE_026_FORTRAN,
      {    NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000,    NS,    NS,    NS, 02102,    NS,    NS, 00042, /*  !"#$%&' */
        01042, 04042, 02042, 04000, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001,    NS,    NS,    NS, 00102,    NS,    NS, /* 89:;<=>? */
           NS, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001,    NS,    NS,    NS,    NS,    NS, /* XYZ[\]^_ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* `abcdefg */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* hijklmno */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* pqrstuvw */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*  ¡¢£¤¥¦§ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¨©ª«¬­®¯ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* °±²³´µ¶· */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¸¹º»¼½¾¿ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÀÁÂÃÄÅÆÇ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÈÉÊËÌÍÎÏ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÐÑÒÓÔÕÖ× */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ØÙÚÛÜÝÞß */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* àáâãäåæç */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* èéêëìíîï */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ðñòóôõö÷ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS  /* øùúûüýþÿ */
      }
    },
    /* Original S/360 EBCDIC. */
    { "ebcdic", "S/360 EBCDIC", PTYPE_029,
      {    NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000, 04006, 00006, 00102, 02102, 01042, 04000, 00022, /*  !"#$%&' */
        04022, 02022, 02042, 04012, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001, 00202, 02012, 04042, 00012, 01012, 01006, /* 89:;<=>? */
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001,    NS,    NS,    NS,    NS, 01022, /* XYZ[\]^_ */
           NS, 05400, 05200, 05100, 05040, 05020, 05010, 05004, /* `abcdefg */
        05002, 05001, 06400, 06200, 06100, 06040, 06020, 06010, /* hijklmno */
        06004, 06002, 06001, 03200, 03100, 03040, 03020, 03010, /* pqrstuvw */
        03004, 03002, 03001,    NS, 04006,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS, 04202,    NS,    NS,    NS,    NS,    NS, /*  ¡¢£¤¥¦§ */
           NS,    NS,    NS,    NS, 02006,    NS,    NS,    NS, /* ¨©ª«¬­®¯ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* °±²³´µ¶· */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¸¹º»¼½¾¿ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÀÁÂÃÄÅÆÇ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÈÉÊËÌÍÎÏ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÐÑÒÓÔÕÖ× */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ØÙÚÛÜÝÞß */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* àáâãäåæç */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* èéêëìíîï */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ðñòóôõö÷ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS  /* øùúûüýþÿ */
      }
    },
    /* 026 keypunch commercial. */
    { "bcd-a", "026 commercial", PTYPE_026_COMMERCIAL,
      {    NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000,    NS,    NS, 00102, 02102, 01042, 04000,    NS, /*  !"#$%&' */
           NS,    NS, 02042,    NS, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001,    NS,    NS,    NS,    NS,    NS,    NS, /* 89:;<=>? */
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001,    NS,    NS,    NS,    NS,    NS, /* XYZ[\]^_ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* `abcdefg */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* hijklmno */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* pqrstuvw */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS, 04042,    NS,    NS,    NS, /*  ¡¢£¤¥¦§ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¨©ª«¬­®¯ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* °±²³´µ¶· */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¸¹º»¼½¾¿ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÀÁÂÃÄÅÆÇ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÈÉÊËÌÍÎÏ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÐÑÒÓÔÕÖ× */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ØÙÚÛÜÝÞß */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* àáâãäåæç */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* èéêëìíîï */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ðñòóôõö÷ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS  /* øùúûüýþÿ */
      }
    },
    /* IBM 1401. */
    { "1401", "IBM 1401", PTYPE_NONPRINTING,
      {    NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000, 03002, 01006, 00102, 02102, 01042, 04000, 01012, /*  !"#$%&' */
        04022, 02022, 02042,    NS, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001, 00022, 02012, 04012, 01022, 00012, 05000, /* 89:;<=>? */
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001,    NS,    NS,    NS,    NS,    NS, /* XYZ[\]^_ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* `abcdefg */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* hijklmno */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* pqrstuvw */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS, 04042,    NS,    NS, 04006, /*  ¡¢£¤¥¦§ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¨©ª«¬­®¯ */
           NS, 01202,    NS,    NS,    NS,    NS,    NS,    NS, /* °±²³´µ¶· */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ¸¹º»¼½¾¿ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÀÁÂÃÄÅÆÇ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÈÉÊËÌÍÎÏ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ÐÑÒÓÔÕÖ× */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ØÙÚÛÜÝÞß */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* àáâãäåæç */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* èéêëìíîï */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* ðñòóôõö÷ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* øùúûüýþÿ */
      }
    },
    /* DEC 026 (historical, no lowercase) */
    { "dec026", "DEC 026 ASCII", PTYPE_026_FORTRAN,
      {
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000, 04006, 01022, 01012, 02102, 01006, 02006, 00012, /*  !"#$%&' */
        01042, 04042, 02042, 04000, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001, 02202, 01202, 04012, 00102, 02012, 04202, /* 89:;<=>? */
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001, 02022, 00006, 04022, 00022, 00202, /* XYZ[\]^_ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* `abcdefg */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* hijklmno */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* pqrstuvw */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
      }
    },
    /* DEC 029 (historical, no lowercase). */
    { "dec029", "DEC 029 ASCII", PTYPE_029,
      {
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
        00000, 04006, 00006, 00102, 02102, 01042, 04000, 00022, /*  !"#$%&' */
        04022, 02022, 02042, 04012, 01102, 02000, 04102, 01400, /* ()*+,-./ */
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, /* 01234567 */
        00002, 00001, 00202, 02012, 02042, 00012, 01012, 01006, /* 89:;<=>? */
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, /* @ABCDEFG */
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, /* HIJKLMNO */
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, /* PQRSTUVW */
        01004, 01002, 01001, 04202, 01202, 02202, 02006, 01022, /* XYZ[\]^_ */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* `abcdefg */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* hijklmno */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* pqrstuvw */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /* xyz{|}~  */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
      }
    },
    /* DEC 026 with control characters and lowercase. */
    { "xdec026", "DEC 026 with lowercase and control chars",
	PTYPE_026_FORTRAN,
      {
        05403, 04401, 04201, 04101, 00005, 01023, 01013, 01007, 
        02011, 04021, 01021, 04103, 04043, 04023, 04013, 04007, 
        06403, 02401, 02201, 02101, 00043, 00023, 00201, 01011, 
        02003, 02403, 00007, 01005, 02043, 02023, 02013, 02007, 
        00000, 04006, 01022, 01012, 02102, 01006, 02006, 00012, 
        01042, 04042, 02042, 04000, 01102, 02000, 04102, 01400, 
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, 
        00002, 00001, 02202, 01202, 04012, 00102, 02012, 04202, 
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, 
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, 
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, 
        01004, 01002, 01001, 02022, 00006, 04022, 00022, 00202, 
        00402, 05400, 05200, 05100, 05040, 05020, 05010, 05004, 
        05002, 05001, 06400, 06200, 06100, 06040, 06020, 06010, 
        06004, 06002, 06001, 03200, 03100, 03040, 03020, 03010, 
        03004, 03002, 03001, 05000, 06000, 03000, 03400, 04005, 
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
      }
    },
    /* DEC 029 with control characters and lowercase. */
    { "xdec029", "DEC 029 with lowercase and control chars", PTYPE_029,
      {
        05403, 04401, 04201, 04101, 00005, 01023, 01013, 01007, 
        02011, 04021, 01021, 04103, 04043, 04023, 04013, 04007, 
        06403, 02401, 02201, 02101, 00043, 00023, 00201, 01011, 
        02003, 02403, 00007, 01005, 02043, 02023, 02013, 02007, 
        00000, 04006, 00006, 00102, 02102, 01042, 04000, 00022,
        04022, 02022, 02042, 04012, 01102, 02000, 04102, 01400,
        01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004,
        00002, 00001, 00202, 02012, 02042, 00012, 01012, 01006,
        00042, 04400, 04200, 04100, 04040, 04020, 04010, 04004, 
        04002, 04001, 02400, 02200, 02100, 02040, 02020, 02010, 
        02004, 02002, 02001, 01200, 01100, 01040, 01020, 01010, 
        01004, 01002, 01001, 04202, 01202, 02202, 02006, 01022,
        00402, 05400, 05200, 05100, 05040, 05020, 05010, 05004, 
        05002, 05001, 06400, 06200, 06100, 06040, 06020, 06010, 
        06004, 06002, 06001, 03200, 03100, 03040, 03020, 03010, 
        03004, 03002, 03001, 05000, 06000, 03000, 03400, 04005, 
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
           NS,    NS,    NS,    NS,    NS,    NS,    NS,    NS, /*          */
      }
    },
    { NULL }
};

#define FROM_CS(cs)	(struct charset *)(void *)(cs)
#define TO_CS(cs)	(charset_t)(void *)(cs)

/* Accessors. */
const char *
charset_name(charset_t cs)
{
    struct charset *c = FROM_CS(cs);

    return c->name;
}

const char *
charset_desc(charset_t cs)
{
    struct charset *c = FROM_CS(cs);

    return c->description;
}

punch_type_t
charset_punch_type(charset_t cs)
{
    struct charset *c = FROM_CS(cs);

    return c->punch_type;
}

unsigned
charset_xlate(charset_t cs, unsigned char ix)
{
    struct charset *c = FROM_CS(cs);

    return c->charset[ix];
}

/*
 * Match a character set by name.
 * Returns NULL for failure.
 */
charset_t
find_charset(const char *name)
{
    int i;

    for (i = 0; charsets[i].name != NULL; i++) {
	if (!strcmp(name, charsets[i].name)) {
	    return TO_CS(&charsets[i]);
	}
    }
    return NULL;
}

/*
 * Return the default character set.
 */
charset_t
default_charset(void)
{
    return TO_CS(&charsets[0]);
}

/*
 * Return the first/next character set.
 * Use NULL to start.
 */
charset_t
next_charset(charset_t curr)
{
    struct charset *c;

    if (curr == NULL) {
	return TO_CS(&charsets[0]);
    }
    c = FROM_CS(curr);
    if (c->name == NULL || (++c)->name == NULL) {
	return NULL;
    }
    return TO_CS(c);
}

#ifdef DUMP_TABLE
void
dump_charset(charset_t cs)
{
    struct charset *c = FROM_CS(cs);
    int i;

    for (i = 0; i < 257; i++) {
	if (i && !(i % 8)) {
	    int j;

	    printf(", /* ");
	    for (j = i - 8; j < i; j++) {
		if ((j & 0x7f) > ' ' && j != 0x7f) {
		    putchar(j);
		} else {
		    putchar(' ');
		}
	    }
	    printf(" */\n");
	}
	if (i == 257) {
	    break;
	}
	if (i % 8) {
	    printf(", ");
	}
	if (c->charset[i] == NS) {
	    printf("   NS");
	} else {
	    printf("0%04o", c->charset[i]);
	}
    }
}
#endif
