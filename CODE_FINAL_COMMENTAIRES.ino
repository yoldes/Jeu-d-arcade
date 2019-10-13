#include <Adafruit_NeoPixel.h>
int numled = 30;
#define PIN      5
#define PINSC      6

int trigPin1 = 11; //pins pour les capteurs à ultrasons
int echoPin1 = 10;
int trigPin2 = 20;
int echoPin2 = 21;

unsigned long duree,test; //variable utilisée pour mesurer le temps d'exécution

boolean starte, serv = 0;
int temps;
int scorej1, scorej2, set1, set2 = 0;//scores et nombre de sets gagnés
int listevitesse[] = {1, 2, 3};
int vitesse = listevitesse[2];
int capteur1, capteur2; //état du capteur du joueur 1 ou 2
int moment = 0; //variable qui détermine le moment où la "balle" repart quand on la renvoie

Adafruit_NeoPixel ruban = Adafruit_NeoPixel(numled, PIN, NEO_GRB + NEO_KHZ800); //déclaration des rubans
Adafruit_NeoPixel rubanscore = Adafruit_NeoPixel(15, PINSC, NEO_GRB + NEO_KHZ800);

uint32_t rouge = ruban.Color(50, 0, 0);
uint32_t vert = rubanscore.Color(0, 50, 0);
uint32_t jaune = rubanscore.Color(50, 50, 0);

void setup() {
  pinMode(trigPin1, OUTPUT); 
  pinMode(echoPin1, INPUT); 
  pinMode(trigPin2, OUTPUT); 
  pinMode(echoPin2, INPUT); 
  ruban.begin();
  rubanscore.begin();
  rubanscore.clear();
  rubanscore.show();
  Serial.begin(9600);
}

boolean gestionscore(boolean joueur, int score) { //on entre 1(true) pour j1, false pour j2, renvoie 1 si le j1 sert, 0 sinon (le j ayant remporté le point précédent)
  if (score >= 5) { //score pour gagner un set : 5
    scorej1 = 0;//remise à zéro des scores
    scorej2 = 0;
    set1 += joueur; //incrémentation du nombre de sets gagnés pour chaque joueur
    set2 += (1 - joueur);
    rubanscore.clear(); //on efface le ruban
    if (set1 == 2 or set2 == 2) { //lorsqu'un j gagne la partie, on allume son côté en vert pendant 1 seconde
      for (int i = ((1 - joueur) * 7); i <= ((1 - joueur) * 7) + 7; i++) {
        rubanscore.setPixelColor(i, vert);
        rubanscore.show();
      }
      delay(1000);
      for (int i = ((1 - joueur) * 7); i <= ((1 - joueur) * 7) + 7; i++) {
        rubanscore.setPixelColor(i, 0);
        rubanscore.show();
      }
      delay(1000);
      set1 = 0;
      set2 = 0;
    }
    else { //lorsqu'un j gagne un set, on allume une led en jaune de son côté
      rubanscore.setPixelColor(5, jaune * set1);
      rubanscore.setPixelColor(9, jaune * set2);
      rubanscore.show();
      return joueur;
    }
  }
  else { 
    rubanscore.setPixelColor(15 * joueur - (15 - score)*pow(-1, joueur + 1) - 1 * joueur, vert); //affichage du score en vert
    rubanscore.show();
    return joueur;
  }
}

boolean distance(boolean capteur) {//on choisit le capteur avec lequel on mesure la distance (1 pour j1)
  long duration;
  int trigPin, echoPin, distance;
  if (capteur) {
    trigPin = trigPin1; //on assigne les pins correspondant au capteur choisi
    echoPin = echoPin1;
  }
  else {
    trigPin = trigPin2;
    echoPin = echoPin2;
  }
  duree = micros(); //mesure du temps d'exécution de la mesure de distance (déclenchement du "chronomètre")
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);// envoie une impulsion de 10ms sur trigPin
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);// lecture de echoPin, renvoie le temps de retour de l'impulsion en microsecondes
  distance = duration * 0.034 / 2;// Calcul de la distance
  duree = micros() - duree; //arrêt du "chronomètre", on a la différence en microsecondes

  if (duree > 10000) { //si le temps dépasse 10ms, on considère que le joueur n'a pas la main devant le capteur
    duree = 10000;
    return 0; 
  }
  else {
    return (distance < 30);
  }
}

boolean lecture(boolean capteur, int led, uint32_t couleur) { //fonction qui lit l'état d'un capteur et allume une led pendant la lecture
  boolean ok = 0;
  ruban.setPixelColor(led, couleur);
  ruban.show();
  for (int i = 0; i <= vitesse - 1; i++) { //on lit n fois l'état du capteur en fonction de la vitesse (de 1 à 3 fois)
    ok += (distance(capteur));
    temps = (vitesse * 10) - (duree / 1000); //on complète le délai pour avoir 10ms entre chaque mesure de distance
    delay(temps/vitesse);
  }

  ruban.setPixelColor(led, 0); //on éteint la led et on renvoie l'état du capteur
  ruban.show();
  return ok;
}

void service(boolean joueur) { //début de partie, on entre 1(true) pour joueur 1
  int led;
  if (joueur) {
    led = 0;
  }
  else {
    led = numled - 1;
  }
  while (not starte) { //la balle clignote pour le joueur choisi jusqu'a ce qu'il appuie sur le bouton
    starte = lecture(joueur, led, rouge);
    delay(30);
  }
}

boolean echange12() { //première moitié du jeu, on renvoie 1 pour que le jeu continue et 0 si le joueur perd
  for (int led = moment + 1; led < numled - 3; led++) {
    capteur2 = lecture(0, led, ruban.Color(50, 75 - 5 * abs(led - 14), 0)); //on teste l'état du capteur j2 et on allume les leds successivement avec un dégradé de couleur
    if (capteur2) {//si on a la main devant le capteur, on perd et l'autre joueur gagne un point
      scorej1++;
      serv = gestionscore(1, scorej1);
      return 0; 
    }
  }
  for (int i = 2; i >= 0; i--) { //sur les 3 dernières leds, la balle est renvoyée avec plus ou moins de vitesse
    capteur2 = lecture(0, numled - i - 1, rouge);
    if (capteur2) { //il faut avoir la main devant le capteur pour continuer l'échange
      vitesse = listevitesse[i];
      moment = i; //détermine l'avance de la balle sur le prochain échange
      return 1;
    }
  }
  scorej1++; 
  serv = gestionscore(1, scorej1);
  return 0;
}

boolean echange21() { //comme précédemment, dans l'autre sens (j2 -> j1)
  for (int led = numled - moment - 1; led > 2; led--) {
    capteur1 = lecture(1, led, ruban.Color(50, 75 - 5 * abs(led - 14), 0));
    if (capteur1) {
      scorej2++;
      serv = gestionscore(0, scorej2);
      return 0;
    }
  }
  for (int i = 2; i >= 0; i--) {
    capteur1 = lecture(1, i, rouge);
    if (capteur1) {
      vitesse = listevitesse[i];
      moment = i; 
      return 1;
    }
  }
  scorej2++;
  serv = gestionscore(0, scorej2);
  return 0;
}

void jeu() {
  service(serv);
  if (serv) { //serv = 1 signifie que le j1 sert
    while (echange12() and echange21()) {//
    }
  }
  else {
    while (echange21() and echange12()) {
    }
  }
  starte = 0; //on revient au service
  vitesse = listevitesse[2];
  moment = 0;
  Serial.print(scorej1); //affichage des scores
  Serial.print(";");
  Serial.println(scorej2);
  delay(500); //délai pour éviter de déclencher le jeu une nouvelle fois sans le vouloir
}

void loop() {
  jeu();
}
