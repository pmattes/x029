/*
 * Copyright (C) 1995 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x026 strange clicking noises
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

extern char *malloc();

static int dfd = -1;

struct aa {
    Audio_hdr hp;
    int asize;
    char *sound;
} loud, soft;

void
audio_init()
{
	if (audiodev_init() < 0)
		return;
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
audiodev_init()
{
	if ((dfd = open(DEV, O_WRONLY)) < 0) {
		perror(DEV);
		return -1;
	}
	return 0;
}

int
audiofile_init(fn, aa)
char *fn;
struct aa *aa;
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
	} else
		aa->asize = aa->hp.data_size;
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
	if (nr < aa->asize)
		aa->asize = nr;
	close(afd);
	return 0;
}

void
loud_click()
{
	if (dfd < 0) {
		fprintf(stderr, "click: audio file not initialized\n");
		return;
	}
	audio_drain(dfd, FALSE);
	(void) write(dfd, loud.sound, loud.asize);
}

void
soft_click()
{
	if (dfd < 0) {
		fprintf(stderr, "click: audio file not initialized\n");
		return;
	}
	audio_drain(dfd, FALSE);
	(void) write(dfd, soft.sound, soft.asize);
}
