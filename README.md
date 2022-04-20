# Super-resolution

Les différentes fonctions d'interpolation de la super-résolution sont appelés dans le fichier main_nano.cu.
Dans ce fichier le choix de la taille de résolution est fait à partir de la variable "resolution".
Puis dans la boucle while(1), sont appelés les différentes fonctions d'interpolation.
De 1 à 3, les fonctions appelés sont les fonctions "Plus Proche Voisin","Bilinéaire" et "Bicubique en C++.
Puis de 4 à 6, ces mêmes fonctions sont appelés mais pour utiliser le GPU de la carte. Ils sont en Cuda et sont nommés "PPV_GPU","bili_GPU" et "bicubic_GPU".
Et enfin 7 appelle une fonction permettant d'échantillonner une image capturée de la caméra.

Pour les fonction en C++, le fichier "mylib.h" représente le fichier header où sont stockés les prototypes des fonctions en C++.

Les fonctions en elles-mêmes sont déclarées dans le fichier "mylib.cpp".
La fonction "affichage-centre" permet d'afficher le centre d'une image si celle si est trop grande à cause d'un facteur de résolution trop élevé.
La fonction "Plus Proche Voisin" est fonctionnelle. Elle copie seulement les pixels connus sur les nouveaux pixels inconnus sur la longeur de l'image avec les variables "vj" et en hauteur avec "vi".
La fonction "Bicubique" est elle aussi focntionnelle, elle sur-échantillonne l'image capturée appelée "image_BR" et devient "image_HR" qui sera l'image sur laquelle les opérations finalles seront effectuées et "image_HR_bis" qui sera une simple copie sur laquelle se fera la détection des pixels connus et les opérations intermédiaires.
Puis de la ligne 105 à 150 est fait de le parcours de "image_HR_bis" pour détecter d'abord un pixel inconnu qui sera dans la variable "P", puis il sera parcouru pour rechercher les pixels connus autour qui seront ranger dans le tableau "tab_ref". Ils seront rangés 4 par 4 car ils correspondent chacuns à une ligne différente.
Puis de la ligne 151 à 162 on cherche à savoir si des pixels connus ont été détectées sur la même ligne ou la même colonne que le pixel inconnu.
Puis de la ligne 163 à 234, si aucun pixel connu est sur la même ligne ou colonne que le pixel inconnu, on effectue une interpolation cubique sur chaque ligne du tableau "tab_ref" à la colonne J. Le résultat est rangé dans la variable "tab_ref1". 
Puis lorque les 4 inteprolations ont été faites, de la ligne 236 à 251 est faite l'interpolation sur l'axe de la colonne avec les anciens résultats à ligne I.
Ligne 252 à 259, remise à 0 des pixels sur "image_HR_bis".
De la ligne 260 à 305 est effectuée une simple interpolations avec les pixels qui sont sur la même ligne ou même colonne que le pixel inconnu.
La fonction "bilineaire" est décalée par rapport à la fonction "bili_GPU", elle reprend le même fonctionement pour trouver les pixels connus autour du pixel inconnu.
Puis calcul de la distance entre les 4 pixels connus pour calculer la moyenne de la valeur du pixel inconnu.

Pour les fonction en Cuda, les prototypes sont dans le fichier "mylib.cuh".

Les fonctions en elles-mêmes sont déclarées dans le fichier "mylib.cu"
Chaque fonction est déclarée dans une fonction __global__void puis est la fonction de type Mat.                                        _______ 
Ces fonctions utilisent un tableau sur une dimension pour manipuler les coordonnées des images.
La fonction "PPV_GPU" est fonctionnelle, le principe est le même que pour la fonction "Plus Proche Voisin" sauf qu'il est 3 fois pour le faire sur chaque couleur du pixel.
La fonction "bili_GPU" est fonctionnelle et utilise le même principe que la fonction "bilineaire".
La fonction "bicubic_GPU" n'est pas fonctionnelle, elle essaye d'utiliser le même principe que sa fonction en C.


