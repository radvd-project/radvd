/*
 *   $Id: timer.c,v 1.2 1997/10/14 20:35:18 lf Exp $
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

static unsigned long sched_timer = 0;

static void alarm_handler(int sig);

void
set_timer(struct timer_lst *tm, int secs)
{
	struct timeval tv;
	struct timer_lst *lst;
	sigset_t bmask, oldmask;

	dlog(LOG_DEBUG, 3, "setting timer: %d secs", secs);

	/*
	 * disable delivery of alarm signals
	 */
	sigemptyset(&bmask);
	sigaddset(&bmask, SIGALRM);

	sigprocmask(SIG_BLOCK, &bmask, &oldmask);

	gettimeofday(&tv, NULL);
	tm->expires = tv.tv_sec + secs;
	
	lst = &timers_head;

	do {
		lst = lst->next;
	} while (tm->expires > lst->expires);

	tm->next = lst;
	tm->prev = lst->prev;
	lst->prev = tm;
	tm->prev->next = tm;

	if (sched_timer == 0 || sched_timer > tm->expires)
	{		
		signal(SIGALRM, alarm_handler);
		sched_timer = tm->expires;		
		dlog(LOG_DEBUG, 4, "calling alarm: %d secs", secs);
		alarm(secs);
	}

	/*
	 * reenable alarm signals
	 */
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

unsigned long
clear_timer(struct timer_lst *tm)
{
	sigset_t bmask, oldmask;
	unsigned long scheduled;

	/*
	 * disable delivery of alarm signals
	 */
	sigemptyset(&bmask);
	sigaddset(&bmask, SIGALRM);

	sigprocmask(SIG_BLOCK, &bmask, &oldmask);
	
	scheduled = tm->expires;
		
	tm->prev->next = tm->next;
	tm->next->prev = tm->prev;
	
	tm->prev = tm->next = NULL;
	
	tm = timers_head.next;

	if (tm != &timers_head)
	{
		struct timeval tv;
		long secs;
	
		gettimeofday(&tv, NULL);
		secs = tv.tv_sec - tm->expires;
		sched_timer = tm->expires;

		if (secs < 0)
			secs = 0;

		dlog(LOG_DEBUG, 4, "calling alarm: %d secs", secs);
		alarm(secs);
	}
	else
		sched_timer = 0;

	/*
	 * reenable alarm signals
	 */
	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	return scheduled;
}

static void
alarm_handler(int sig)
{
	struct timer_lst *tm, *back;
	struct timeval tv;

	tm = timers_head.next;

	gettimeofday(&tv, NULL);

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

	if (tm != &timers_head)
	{
		long secs;
	       
		secs = tm->expires - tv.tv_sec;
		sched_timer = tm->expires;

		if (secs <= 0)
		{
			secs = 1;
		}

		signal(SIGALRM, alarm_handler);
		dlog(LOG_DEBUG, 4, "calling alarm: %d secs", secs);
		alarm(secs);
	}
	else
		sched_timer = 0;
}

void
init_timer(struct timer_lst *tm, void (*handler)(void *), void *data)
{
	memset(tm, 0, sizeof(struct timer_lst));
	tm->handler = handler;
	tm->data = data;
}
