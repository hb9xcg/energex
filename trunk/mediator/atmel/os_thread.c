/*
 * Energex
 *
 * Copyright (C) 2005-2007 by Timo Sandmann
 * mail@timosandmann.de
 *
 * Copyright (C) 2008-2010 by Markus Walser
 * markus.walser@gmail.com
 * 
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version. 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 * 
 */

#include "mediator.h"
#include "os_thread.h"
#include "os_utils.h"
#include <stdlib.h>

/*! Datentyp fuer Instruction-Pointer */
typedef union {
	void * ip;			/*!< Instruction-Pointer des Threads */
	struct {
		uint8_t lo8;	/*!< untere 8 Bit der Adresse */
		uint8_t hi8;	/*!< obere 8 Bit der Adresse */
	};
} os_ip_t;

Tcb_t os_threads[OS_MAX_THREADS];	/*!< Array aller TCBs */
Tcb_t * os_thread_running = NULL;	/*!< Zeiger auf den TCB des Threads, der gerade laeuft */

/*!
 * Legt einen neuen Thread an.
 * Der zuerst angelegt Thread bekommt die hoechste Prioritaet,
 * je spaeter ein Thread erzeugt wird, desto niedriger ist seine
 * Prioritaet, das laesst sich auch nicht mehr aendern!
 * @param *pStack	Zeiger auf den Stack (Ende!) des neuen Threads
 * @param *pIp		Zeiger auf die Main-Funktion des Threads (Instruction-Pointer)
 * @return			Zeiger auf den TCB des angelegten Threads
 */
Tcb_t * os_create_thread(uint8_t * pStack, void * pIp) {
	os_enterCS();	// Scheduler deaktivieren
	uint8_t i;
	Tcb_t * ptr = os_threads;
	// os_scheduling_allowed == 0 wegen os_enterCS()

	for (i=os_scheduling_allowed; i<OS_MAX_THREADS; i++, ptr++) {	
		if (ptr->stack == NULL) {
			if (os_thread_running == NULL) {
				/* Main-Thread anlegen (laeuft bereits) */
				os_thread_running = ptr;
				ptr->stack = pStack;
			} else {
				/* "normalen" Thread anlegen */
				os_ip_t tmp;
				tmp.ip = pIp;
				/* Return-Adresse liegt in Big-Endian auf dem Stack! */
				*pStack = tmp.lo8;
				*(pStack-1) = tmp.hi8;
				tmp.ip = &os_exitCS;
				*(pStack-2) = tmp.lo8;
				*(pStack-3) = tmp.hi8;
				ptr->stack = pStack-4;	// 4x push => Stack-Pointer - 4
			}
			os_scheduling_allowed = 1;	// Scheduling wieder erlaubt
			/* TCB zurueckgeben */
			return ptr;
		}
	}
	os_scheduling_allowed = 1;	// Scheduling wieder erlaubt
	return NULL;	// Fehler :(
}

/*!
 * Beendet den kritischen Abschnitt wieder, der mit enterCS began
 */
void os_exitCS(void) {
	if (test_and_set((uint8_t *)&os_scheduling_allowed, 1) == 2) {
		os_schedule(TIMER_GET_TICKCOUNT_32);
	}
}

/*!
 * Schaltet auf den Thread mit der naechst niedrigeren Prioritaet um, der lauffaehig ist,
 * indem diesem der Rest der Zeitscheibe geschenkt wird.
 */
void os_thread_yield(void) {
	uint32_t now = TIMER_GET_TICKCOUNT_32;
	/* Zeitpunkt der letzten Ausfuehrung nur in 8 Bit, da yield() nur Sinn macht, wenn es
	 * regelmaessig aufgerufen wird und eine Aufloesung im ms-Bereich gebraucht wird. 
	 * Sonst sleep() benutzen! */
	uint8_t diff = (uint8_t)now - os_thread_running->lastSchedule;
	if (diff > MS_TO_TICKS(OS_TIME_SLICE)) {
		/* Zeitscheibe wurde ueberschritten */
		os_thread_running->nextSchedule = now;
	} else {
		/* Wir haben noch Zeit frei, die wir dem naechsten Thread schenken koennen */
		os_thread_running->nextSchedule = now - (uint8_t)(diff + (uint8_t)MS_TO_TICKS(OS_TIME_SLICE));
	}
	/* Scheduler wechselt die Threads */
	os_schedule(now);
}

/*!
 * Schaltet "von aussen" auf einen neuen Thread um. 
 * => kernel threadswitch
 * Achtung, es wird erwartet, dass Interrupts an sind. 
 * Sollte eigentlich nur vom Scheduler aus aufgerufen werden!
 * @param *from	Zeiger auf TCB des aktuell laufenden Threads
 * @param *to	Zeiger auf TCB des Threads, der nun laufen soll
 */
void os_switch_thread(Tcb_t * from, Tcb_t * to) {
	os_thread_running = to;
	/* r0, r1, r18 bis r27, Z und SREG werden hier nach folgender Ueberlegung NICHT gesichert:
	 * r0, r18 bis r27 und Z wurden bereits vor dem Eintritt in diese Funktion auf dem (korrekten) Stack gesichert!
	 * Falls wir aus der Timer-ISR kommen, wurden auch r1 und das Statusregister bereits gerettet. Falls nicht, 
	 * duerfen wir das SREG ruhig ueberschreiben, weil der (noch) aktuelle Thread in diesem Fall den Scheduler explizit 
	 * aufrufen wollte, der Compiler also weiss, dass sich das Statusregister aendern kann (kooperativer Threadswitch).
	 * In r1 muss vor jedem Funktionsaufruf bereits 0 stehen.  
	 */
	asm volatile(
		"push r2				; save GP registers	\n\t"
		"push r3									\n\t"
		"push r4									\n\t"
		"push r5									\n\t"
		"push r6									\n\t"
		"push r7									\n\t"
		"push r8									\n\t"
		"push r9									\n\t"
		"push r10									\n\t"
		"push r11									\n\t"
		"push r12									\n\t"
		"push r13									\n\t"
		"push r14									\n\t"
		"push r15									\n\t"
		"push r16									\n\t"
		"push r17									\n\t"
	//-- hier ist noch "from" (Z) der aktive Thread 	--//
		"call os_switch_helper	; switch stacks		\n\t"
	//-- jetzt ist schon "to" (Y) der aktive Thread 	--//
		"pop r17				; restore registers	\n\t"
		"pop r16									\n\t"			
		"pop r15									\n\t"
		"pop r14									\n\t"
		"pop r13									\n\t"
		"pop r12									\n\t"
		"pop r11									\n\t"
		"pop r10									\n\t"			
		"pop r9										\n\t"			
		"pop r8										\n\t"			
		"pop r7										\n\t"			
		"pop r6										\n\t"			
		"pop r5										\n\t"			
		"pop r4										\n\t"			
		"pop r3										\n\t"			
		"pop r2											"			
		::	"y" (&to->stack), "z" (&from->stack)	// Stackpointer
		:	"memory"
	);
}

/*
 * Hilfsfunktion fuer Thread-Switch, nicht beliebig aufrufbar!
 * (Erwartet Zeiger auf TCBs in Z und Y).
 * Als extra Funktion implementiert, um die Return-Adresse auf dem Stack zu haben.
 */
void os_switch_helper(void) {
	//-- coming in as "Z" --//
	asm volatile(
		"cli				; Interrupts off		\n\t"			
		"in r16, __SP_L__	; switch Stacks			\n\t"
		"st Z+, r16									\n\t"	
		"in r16, __SP_H__							\n\t"
		"st Z, r16									\n\t"	
			//-- live changes here --//
		"ld r16, Y+ 								\n\t"
		"out __SP_L__, r16							\n\t"
		"ld r16, Y 									\n\t"
		"out __SP_H__, r16							\n\t"
		"sei				; Interrupts on				"
		::: "memory"
	);
	//-- continue as "Y" --//
}

