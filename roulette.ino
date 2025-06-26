#define ulong_t unsigned long

#define SPEAKER 12
#define ROLL_BUTTON 11

#define PLAYER_COUNT 4
#define BARREL_SIZE 6

bool started = false;
bool alive[PLAYER_COUNT];

uint8_t currentPlayer = 0;
uint8_t bulletPos = 0;

ulong_t lastFlash = 0;
bool flashState = true;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  
  for(size_t i = 0; i < PLAYER_COUNT; i++) {
    uint8_t led = getLed(i);

    pinMode(led, OUTPUT);
    pinMode(ROLL_BUTTON, INPUT_PULLUP);
    pinMode(getBtn(i), INPUT_PULLUP);
  }
}

void setAllLED(bool state) {
  for(size_t i = 0; i < PLAYER_COUNT; i++) {
    digitalWrite(getLed(i), state);
  }
}

void setAliveLED(bool state) {
  for(size_t i = 0; i < PLAYER_COUNT; i++) {
    if(!alive[i]) continue;

    digitalWrite(getLed(i), state);
  }
}

void rollBarrel() {
  bulletPos = random(0, BARREL_SIZE - 1);

  const ulong_t del = 25;

  for(size_t i = 0; i < PLAYER_COUNT * random(2, 5); i++) {
    digitalWrite(getLed(i % PLAYER_COUNT), HIGH);
    tone(SPEAKER, 200);

    delay(del);
    digitalWrite(getLed(i % PLAYER_COUNT), LOW);
    noTone(SPEAKER);

    delay(del);
  }
}

void rollPlayer() {
  size_t len = random(10, 20);

  for(size_t i = random(0, PLAYER_COUNT - 1); i < len; i++) {
    uint8_t led = getLed(i % PLAYER_COUNT);
    ulong_t del = 5 * i;

    tone(SPEAKER, 300 * (i % 3));
    digitalWrite(led, HIGH);
    delay(del);

    if(i != len - 1) {
      digitalWrite(led, LOW);
    } else {
      currentPlayer = i % PLAYER_COUNT;
      delay(del);
    }

    noTone(SPEAKER);
    delay(del);
  }

  delay(500);
  selectPlayer(currentPlayer);
}

void selectPlayer(uint8_t player) {
  setAllLED(false);
  delay(50);
  digitalWrite(getLed(player), true);

  tone(SPEAKER, 800, 100);
  currentPlayer = player;

  delay(200);
}

void selectNextPlayer() {
  for(size_t offset = 1; offset < PLAYER_COUNT; offset++) {
    size_t player = (currentPlayer + offset) % PLAYER_COUNT;

    if(alive[player]) {
      selectPlayer(player);
      break;
    }
  }
}

uint8_t getAliveCount() {
  uint8_t res = 0;

  for(size_t i = 0; i < PLAYER_COUNT; i++) {
    if(alive[i]) {
      res++;
    }
  }

  return res;
}

int getPressedPlayer() {
  for(size_t i = 0; i < PLAYER_COUNT; i++) {
    if(!digitalRead(getBtn(i))) {
      Serial.println(i);
      return i;
    }
  }

  return -1;
}

uint8_t getLed(uint8_t player) {
  return player * 2 + 3;
}

uint8_t getBtn(uint8_t player) {
  return player * 2 + 2;
}

void startGame() {
  started = true;

  for(size_t i = 0; i < PLAYER_COUNT; i++) {
    alive[i] = true;
  }

  tone(SPEAKER, 500);
  delay(200);
  noTone(SPEAKER);
  delay(500);

  rollBarrel();
  delay(500);
  rollPlayer();
}

void loop() {
  if(started) {
    gameLoop();
  } else {
    menuLoop();
  }
}

void menuLoop() {
  ulong_t now = millis();

  if(now > lastFlash + 500) {
    lastFlash = now;
    flashState = !flashState;
    setAllLED(flashState);
  }

  if(getPressedPlayer() != -1) {
    startGame();
  }
}

void gameLoop() {
  ulong_t now = millis();

  if(now > lastFlash + 500) {
    lastFlash = now;
    flashState = !flashState;

    setAliveLED(flashState);
    digitalWrite(getLed(currentPlayer), true);
  }

  int pressedPlr = getPressedPlayer();

  if(!digitalRead(ROLL_BUTTON)) {
    rollBarrel();
    delay(500);
    selectNextPlayer();
    return;
  }

  if(pressedPlr != -1) {
    if(!alive[pressedPlr]) {
      tone(SPEAKER, 100, 200);
      return;
    }

    uint8_t led = getLed(pressedPlr);

    setAllLED(false);
    digitalWrite(led, HIGH);
    digitalWrite(getLed(currentPlayer), HIGH);

    delay(200);

    for(size_t i = 1; i < 5; i++) {
      ulong_t del = 150;
      
      tone(SPEAKER, 100 * i);
      delay(del);
    }

    noTone(SPEAKER);
    delay(500);

    if(bulletPos == 0) {
      digitalWrite(led, LOW);
      ulong_t end = millis() + 200;

      // white noise
      while(end > millis()) {
        ulong_t del = random(50, 500);

        digitalWrite(SPEAKER, HIGH);
        delayMicroseconds(del);
        digitalWrite(SPEAKER, LOW);
        delayMicroseconds(del);
      }
      
      alive[pressedPlr] = false;

    } else {
      tone(SPEAKER, 100);
      delay(100);
      noTone(SPEAKER);
    }

    delay(500);
    digitalWrite(led, LOW);

    bulletPos = (bulletPos + 1) % BARREL_SIZE;
    
    if(getAliveCount() > 1) {
      selectNextPlayer();
    } else {
      uint8_t winner = 0;

      for(size_t i = 0; i < PLAYER_COUNT; i++) {
        if(alive[i]) {
          winner = i;
          break;
        }
      }

      digitalWrite(getLed(winner), HIGH);


      for(size_t i = 1; i <= 4; i++) {
        tone(SPEAKER, 200 * i, 1000);
        delay(200);
      }

      noTone(SPEAKER);
      delay(100);

      tone(SPEAKER, 200);
      delay(100);
      tone(SPEAKER, 800);

      delay(500);
      noTone(SPEAKER);
      
      delay(1000);
      setAllLED(false);
      started = false;
    }
  }
}



