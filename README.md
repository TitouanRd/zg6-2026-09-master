# ZG6 - PARTIE ELECTRONIQUE

## Présentation des projets

Dans le cadre de la ZG6, deux cartes sont fournies. Elles utilisent un bornier Arduino.

**ATTENTION** : les deux projets, bien que similaires, ne sont pas compatibles. **Chaque projet doit utiliser sa propre STM32.**.

* [contrôle de la guitare](guitare.md)
* [traitement du signal du microphone](microphone.md)

Le répertoire contient des ressources utiles (schémas des cartes, datasheets).

## Tâches à réaliser

###  **semaine 1**

**Projet guitare** : assurer la configuration et le contrôle des éléments.

* Prévoir des commandes de base pour réaliser de manière isolée les différentes actions (pincer la corde / la relacher, gratter la corde / arrêter, déplacer le chariot d'une distance correspondant à un certain nombre de pas de rotation du moteur pas à pas -- distance en pas ou en mm).
* Réaliser la commande trapézoïdale en vitesse du moteur pas à pas (voir [step_ctrl.pdf](docs/step_ctrl.pdf)) pour permettre au chariot d'atteindre le plus rapidement possible une position donnée.

### **semaine 2** Contrôler la guitare à partir du PC pour jouer des notes (projet signal)

**Projet guitare** :

* Réaliser une table de correspondance entre déplacement du chariot et note.

**Projet signal** : identifier la fréquence correspondant à la note.

* Compléter le projet signal pour échantillonner le signal du microphone et identifier la fréquence de la note jouée. Afficher le signal et le spectre sur l'écran LCD.


### **semaine 3** Jouer une partition

Le dispositif doit être capable de jouer plusieurs notes, permettant de reconnaître un air connu, et de récupérer une partition à partir d'un PC et/ou d'une carte SD.
