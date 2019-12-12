#include "Sonar.h"
#include "Arduino.h"

/* Problema: esistono casi abbastanza particolari, vedi superfici fonoassorbenti, in cui l'eco ha scarsa 
 *           probabilità di ritornare alla sorgente del segnale. 
 *           pulseInLong() ha un timeout di default di 1 secondo, ovvero: se il segnale non ritorna entra 
 *           un secondo, la chiamata al metodo termina. Questa tempistica è poco adatta al nostro sistema
 *           in quanto induce sicuramente in overrun.
 *           E' stato quindi trovato un tempo di timeout più idoneo:
 *                                          tempo >= velocita_suono * DISTANZA_FAR * 2
 *           Ovvero: velocità del suono, per la distanza massima di interesse, per due, ovvero andata e 
 *           ritorno del segnale. 
 *           Viene ritornata, in caso di interrupt, una distanza sicuramente maggiore di DISTANZA_FAR (0.4 metri) */

Sonar::Sonar(int trigPin, int echoPin){
  this->trigPin = trigPin;
  this->echoPin = echoPin;
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float Sonar::getDistance(){
  /* Send of signal that has to be captured */
  digitalWrite(trigPin,LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin,LOW);
  /* Capture of the signal sent: if TIME_OUT passed it return 0 */
  long tUS = pulseInLong(echoPin, HIGH, TIME_OUT_TIME);
  /* If TIME_OUT_TIME didn't passed, calculate the distance obteined */
  if(tUS != 0){
    double t = tUS / 1000.0 / 1000.0 / 2;
    double d = t*vs;
    return d;
  }
  /* If TIME_OUT_TIME passed, return an invalid value */
  return RETURN_OUT_OF_TIME; 
}
