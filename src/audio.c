/*
 * Copyright (c) 1995-2025 Paul Mattes.
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
 * x029 strange clicking noises
 */

#include <multimedia/libaudio.h>
#include <multimedia/audio_device.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOUD	"click1.au"
#define SOFT	"click2.au"
#define DEV	"/dev/audio"

extern char *malloc(void);

static int dfd = -1;

struct aa {
    Audio_hdr hp;
    int asize;
    char *sound;
} loud, soft;

void
audio_init(void)
{
    if (audiodev_init() < 0) {
	return;
    }
    if (audiofile_init(LOUD, &loud)) {
	close(dfd);
	dfd = -1;
	return;
    }
    if (audiofile_init(SOFT, &soft)) {
	close(dfd);
	dfd = -1;
	return;
    }
}

int
audiodev_init(void)
{
    if ((dfd = open(DEV, O_WRONLY)) < 0) {
	perror(DEV);
	return -1;
    }
    return 0;
}

int
audiofile_init(char *fn, struct aa *aa)
{
    int afd;
    int nr;

    if (audio_isaudiofile(fn) != TRUE) {
	fprintf(stderr, "%s is not an audio file\n", fn);
	return -1;
    }
    if ((afd = open(fn, O_RDONLY)) < 0) {
	perror(fn);
	return -1;
    }
    if (audio_read_filehdr(afd, &aa->hp, 0, 0) != AUDIO_SUCCESS) {
	fprintf(stderr, "Can't read header from %s\n", fn);
	close(afd);
	return -1;
    }
    if (aa->hp.data_size == AUDIO_UNKNOWN_SIZE) {
	struct stat s;

	(void) fstat(afd, &s);
	aa->asize = s.st_size;
    } else {
	aa->asize = aa->hp.data_size;
    }
    if (audio_set_play_config(dfd, &aa->hp) != AUDIO_SUCCESS) {
	fprintf(stderr, "Can't set play configuration\n");
	close(afd);
	return -1;
    }
    if (!(aa->sound = malloc(aa->asize))) {
	fprintf(stderr, "Can't allocate sound memory\n");
	close(afd);
	return -1;
    }
    if ((nr = read(afd, aa->sound, aa->asize)) < 0) {
	perror("sound read");
	close(afd);
	return -1;
    }
    if (nr < aa->asize) {
	aa->asize = nr;
    }
    close(afd);
    return 0;
}

void
loud_click(void)
{
    if (dfd < 0) {
	fprintf(stderr, "click: audio file not initialized\n");
	return;
    }
    audio_drain(dfd, FALSE);
    (void) write(dfd, loud.sound, loud.asize);
}

void
soft_click(void)
{
    if (dfd < 0) {
	fprintf(stderr, "click: audio file not initialized\n");
	return;
    }
    audio_drain(dfd, FALSE);
    (void) write(dfd, soft.sound, soft.asize);
}
