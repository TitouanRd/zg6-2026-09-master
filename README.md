# ZG6 MEDIATOR - PARTIE ELEC

## CONFIGURATION MATERIELLE

### description matérielle


### Liste des broches

* Solénoïde : broche PC8 en GPIO output high speed
* Servo-moteur : commande Timer TIM4 channel 1 en PWM (broche PB6 - AF02)
* Moteur pas à pas
  - commande de vitesse par signal PWM (50%) de fréquence réglable généré par 
    le TIMER TIM1 CH1 (broche PA8 - AF01)
  - DIR : sens de rotation - broche PA9 output
  - EN  : enable - broche PC7 output
  
* Capteur fin de course : broche PC9 en GPIO + IRQ sur front montant/descendant

## Organisation logicielle

Les drivers des périphériques sont disponibles dans le répertoire `lib`.

Le répertoire `src` contient 2 fichiers

- `main.c` qui contient l'application qui est construite autour d'une boucle qui récupère et traite les évènements fournis par les différents périphériques (UART, ADC, STEPPER).
- `hardware.c` qui implémente une API d'utilisation des différents éléments.

Vous êtes libre d'ajouter des fichiers à votre convenance de manière à structurer le projet.

## Fonctionnement de la guitare

Le servomoteur est utilisé pour gratter la corde et ainsi la faire vibrer.

Le solénoïde permet de pincer la corde pour modifier la longueur de corde. On obtient des fréquences de résonance différentes en fonction de cette longueur.

Le positionnement du solenoïde au moment du pincement de la corde est assuré par un moteur pas à pas de type NEMA17.

## Tâches à réaliser

###  **semaine 1** Assurer la configuration et le contrôle des éléments (projet guitare)

- le solénoïde : commande en tout ouu rien
- le servomoteur : il es commandé par un signal PWM de période 20 ms avec un rapport cyclique réglable tel que la largeur de l'impulsion varie de 0.5 ms à 2.5 ms pour obtenir une variation de l'angle de 0 à 180°.
    
- le moteur pas à pas est de type NEMA17 avec 200 pas par tour. Il est commandé par l'intermédiaire d'un driver qui utilise 3 signaux de commande.
  * EN : actif au niveau bas, il permet de valider la commande du moteur.
  * DIR : il permet de choisir un sens de direction
  * STEP : une horloge permettant de piloter l'avancement d'un pas à chaque front.
  
  Le driver est configuré matériellement pour commander le moteur pas à pas par micropas (microstep). On a 32 micropas par pas du moteur, soit 6400 micropas par tour.

  Le moteur est commandé en boucle ouverte (classique pour ce type de moteur). La vitesse de rotation du moteur est liée à la fréquence du moteur par la relation
    
    `n [tr/s] = f [Hz] /6400`
    
  Le moteur a un vitesse de rotation limite au delà de laquelle, il décroche et n'est plus capable d'assurer l'égalité 1 front d'horloge = 1 micropas. Une méthode généralement utilisée pour éviter le risque de glissement est de commander le moteur en faisant varier la vitesse suivant un profil trapézoïdal.
  
  Un capteur de fin de course doit permettre à l'initialisation de venir mettre le chariot comportant le solénoïde en butée, puis de l'en éloigner jusqu'à une position qui servira de référence (position 0) pour les déplacements futurs.
    
  L'API développée doit permettre de positionner le chariot de manière précise.
    
  Une mesure de la vitesse de déplacement linéaire doit être réalisée pour valider la commande. Un plus serait l'implémentation du profil trapézoïdal.
    
  Voir le document [step_ctrl.pdf](docs/step_ctrl.pdf)
    
Pour tester les différents éléments, on mettra en place un protocole de communication avec des commandes qui permettront de contrôler (et tester) les différents éléments du système.

Une démonstration du contrôles des éléments est attendue.

### **semaine 2** Contrôler la guitare à partir du PC pour jouer des notes (projet signal)

L'application microcontrôleur doit permettre, à l'aide d'un microphone d'enregistrer le signal correspondant à une note et d'identifier celle-ci. Un affichage du signal et du spectre serait un plus.
  
Informations sur la bibliothèque [CMSIS-DSP](https://arm-software.github.io/CMSIS_5/DSP/html/group__RealFFT.html#ga3df1766d230532bc068fc4ed69d0fcdc)
  
### **semaine 3** Jouer une partition

  Le dispositif doit être capable de jouer plusieurs notes, permettant de reconnaître un air connu, et de récupérer une partition à partir d'un PC et/ou d'une carte SD.
