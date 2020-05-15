# Dazibao

Dazibao ("journal à grandes lettres") implémente un pair dans un réseau pair à pair permettant de publier une donnée et d'obtenir la liste des données publiées par les autres membres du réseau. Il est basé sur un algorithme d'inondation non-fiable.

### Installation

Vous aurez besoin d'installer les bibliothèque `glib`
- Sur Linux
```bash
sudo apt-get install libglib2.0-dev
```
- Avec HomeBrew sur MacOs :
```bash
brew install glib
```

et `openssl`
- Sur Linux
```bash
sudo apt-get install libssl-dev
```

- Avec HomeBrew sur MacOs :
```bash
 brew install openssl
 ```
Puis le lier manuellement :
 ```bash
 ln -s /usr/local/opt/openssl/include/openssl /usr/local/include
 ```

 ```bash
ln -s /usr/local/Cellar/openssl/[version]/include/openssl /usr/bin/openssl
ln -s /usr/local/opt/openssl/lib/libssl.1.0.0.dylib /usr/local/lib/
```

### Usage
En se plaçant dans le répertoire `src`
```bash
make
```
```bash
./dazibao <hostname> <port>
```
Exemple :
```bash
./dazibao jch.irif.fr 1212
```
Pour lancer avec le mode debug:

```bash
./dazibao <hostname> <port> debug
```

Pour lancer et avoir une vue détaillée des TLVs reçus:

```bash
./dazibao <hostname> <port> details
```


##### Auteurs :
- CHEKROUN Hiba-Asmaa
- MIRET Blanche
