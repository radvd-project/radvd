/*
 *   $Id: timer.c,v 1.3 1999/07/30 11:29:04 lf Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lf@elemental.net>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>

static struct timer_lst timers_head = {
	~0UL,
	NULL, NULL,
	&timers_head, &timers_head
};

static void alarm_handler(int sig);

static void
schedule_timer(struct timer_lst *tm, struct timeval tv *tv)
{
	if (tm != &timers_head)
	{
		int secs;
	       
		secs = tm->expires - tv->tv_sec;

		if (secs <= 0)
			secs = 1;

		signal(SIGALRM, alarm_handler);
		dlog(LOG_DEBUG, 4, "calling alarm: %d secs", secs);
		alarm(secs);
	}
	else
	{
		alarm(0);
	}
}

void
set_timer(struct timer_lst *tm, int secs)
{
	struct timeval tv;
	struct timer_lst *lst;
	sigset_t bmask, oldmask;

	dlog(LOG_DEBUG, 3, "setting timer: %d secs", secs);

	sigemptyset(&bmask);
	sigaddset(&bmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &bmask, &oldmask);

	gettimeofday(&tv, NULL);

	if (secs <= 0)
		secs = 1;

	tm->expires = tv.tv_sec + secs;
	
	lst = &timers_head;

	do {
		lst = lst->next;
	} while (tm->expires > lst->expires);

	tm->next = lst;
	tm->prev = lst->prev;
	lst->prev = tm;
	tm->prev->next = tm;

	schedule_timer(timers_head.next, &tv);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

unsigned long
clear_timer(struct timer_lst *tm)
{
	sigset_t bmask, oldmask;
	unsigned long scheduled;
	struct timeval tv;

	sigemptyset(&bmask);
	sigaddset(&bmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &bmask, &oldmask);
	
	gettimeofday(&tv, NULL);

	scheduled = tm->expires;
		
	tm->prev->next = tm->next;
	tm->next->prev = tm->prev;
	
	tm->prev = tm->next = NULL;
	
	tm = timers_head.next;

	gettimeofday(&tv, NULL);

	schedule_timer(tm, &tv);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	return scheduled;
}

static void
alarm_handler(int sig)
{
	struct timer_lst *tm, *back;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	tm = timers_head.next;

	while (tm->expires <= tv.tv_sec)
	{		
		tm->prev->next = tm->next;
		tm->next->prev = tm->prev;

		back = tm;
		tm = tm->next;
		back->prev = back->next = NULL;

		(*back->handler)(back->data);
	}

	tm = timers_head.next;

	schedule_timer(tm, &tv);
}


void
init_timer(struct timer_lst *tm, void (*handler)(void *), void *data)
{
	memset(tm, 0, sizeof(struct timer_lst));
	tm->handler = handler;
	tm->data = data;
}
