/* Initialisiert den Soft-UART. Vor Verwendung des Soft-UARTs müssen Interrupts 
 * mit sei() global aktiviert worden sein (egal, ob vor oder nach dem Aufruf von
 * uart_init(). 
 */
extern void softuart_init(void);

/* Sendet das Zeichen c. Vorher wird gewartet, bis eine evtl. laufende Sendung
 * beendet ist.
 */
extern void softuart_putc (const char c);

/* Wartet bis zum nächsten Empfang bzw. liefert das empfangene Zeichen im
 * Wertebereich 0...255. 
 */
extern int softuart_getc_wait(void);

/* Schaut nach, ob ein Zeichen empfangen wurde und liefert dieses gegebenenfalls
 * als int zurück (Wertebereich ist 0...255). Wurde nichts empfangen, wird -1
 * geliefert. 
 */
extern int softuart_getc_nowait(void);

