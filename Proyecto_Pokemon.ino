#include <font.h>
#include <ILI9341_8bits_Kurt.h>
#include <lcd_registers.h>
#include <TM4C123GH6PM.h>
#include <SPI.h>
#include <SD.h>

#include "bitmaps.h"

// Colors
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define YELLOW 0xFFE0



// Text
#define TEXT_HEIGHT 16

// Movement
#define TILE_SIZE 16 // Square tiles of 16x16 for all sprites.
#define STEP 16 // Step size to move

// Position
#define LEFT_BORDER 16
#define RIGHT_BORDER 288
#define TOP_BORDER 16
#define BOTTOM_BORDER 224 

// Main Menu
int menu_option = 1;
int menu_select = 0;
int PrintedMainMenu = 0;
int InsideMenu = 0;

// Sprites
int sprite1_pos_x = 0;
int sprite1_pos_y = 0;
int sprite2_pos_x = 0;
int sprite2_pos_y = 0;
int sprite3_pos_x = 0;
int sprite3_pos_y = 0;

int sprite1_prev_x = 0;
int sprite1_prev_y = 0;
int sprite2_prev_x = 0;
int sprite2_prev_y = 0;
int sprite3_prev_x = 0;
int sprite3_prev_y = 0;

const int dataReadyPin = PB_0;
const int chipSelectPin = PB_5;
// Map

// Collisions
int temp_collision = 0;

int lose = 0;
int win = 0;
int lectura;
char Regis[80];
char Regis1[80];


uint8_t P1_Data = 0;  // jugador Control data received via UART
uint8_t P2_Data = 0;

 
uint8_t P1_Control_Valores [3];
uint8_t P2_Control_Valores [3];

 
int P1_Information [] = { 160 - 30, 120, 0b0000, 0b00, 3, 0};
int P2_Information [] = { 160 + 30, 120, 0b0000, 0b00, 3, 0};



int Pok1_Info [] = {40 , 66, 0, 3};   // Información de pokemons
int Pok2_Info [] = {120 , 180, 0, 3};
int Pok3_Info [] = {180 , 85, 0, 3};
int Pok4_Info [] = { 215, 30, 1, 3};
int Pok5_Info [] = { 215, 30, 1, 3};
int Pok6_Info [] = { 215, 30, 1, 3};

//Animation Speeds for moving elements 
int AnimationP1Speed = 0;  
int AnimationP2Speed = 0;   

int AnimationEnemy1Speed = 0;     
int AnimationEnemy2Speed = 0;    
int AnimationEnemy3Speed = 0;     
int AnimationEnemy4Speed = 0;    
int AnimationEnemy5Speed = 0;     
int AnimationEnemy6Speed = 0;    

int AttackP1Speed = 0;      
int AttackP2Speed = 0;      

int P1AttackPosition = 0;   
int P2AttackPosition = 0;   
//Game Status variables
int EnemiesLeft = 6;
int GameWon = 0;
int GameLost = 0;
int AxisChange1 = 0;
int AxisChange2 = 0;

// This bitmaps are stored in the EPRROM by consequence of their large Valores
extern uint8_t Pokemon_camina_lado[];
extern uint8_t Pokemon_arriba [];
extern uint8_t Pokemon_abajo [];
extern uint8_t Mapa [];
extern uint8_t Entrenador_Ataque [];

  File myFile;
  Sd2Card card;
  SdVolume volume;
  SdFile root; 


void setup() {
  
  // Frecuencia de reloj (utiliza TivaWare)
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  
  // Configuración del puerto (utiliza TivaWare)
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

  //
  
    SPI.setModule(0);

  Serial.print("\nInitializing SD card...");
  
  pinMode(PA_3, OUTPUT);     // change this to 53 on a mega

   if (!SD.begin(PA_3)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  

  if (!card.init(SPI_HALF_SPEED, PA_3)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card is inserted?");
    Serial.println("* Is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

  


  

  //
  
  //   Inicialización de la pantalla
  LCD_Init();
  LCD_Clear(0x05F9);

  Serial2.begin(9600);  //Start serial communiPikaion with the peripheral via channel 2 (UART on PD6 and PD7 pins in the launchpad)
  Serial3.begin(96005);  //Start serial communiPikaion with the peripheral via channel 3
 
}



void loop() 
{
  // Print graphics of main menu
  
  print_main_menu();
  P1_Information [3] = P1_Control_Valores [0];
  delay(10);
  if (Serial2.available() > 0) //Check if there is data available in the bus 
     {
        P1_Data = Serial2.read(); // Receive the jugador 1 Control data   
     }
  
      //Now, we process the jugador 1 data received:
      P1_Valor_Controles[0] = (P1_Data & 0b100000) >> 5;
      P1_Valor_Controles[1] = (P1_Data & 0b010000) >> 4;
      P1_Valor_Controles[2] = (P1_Data & 0b001111);

  menu_navigate();

  // Navigate menu options based on pushbutton 1 
  switch (menu_option)
  {
    case 1: // Start Game
      if (InsideMenu == 0)
      {
        pikachub_textbox(80, 85, 160, TEXT_HEIGHT, WHITE, BLUE);
        Rect(100, 135, 112, TEXT_HEIGHT, BLUE);
        Rect(65, 110, 192, TEXT_HEIGHT, BLUE); 
      }
      break;

    case 2: // Instructions
      if (InsideMenu == 0)
      {
        pikachub_textbox(65, 110, 192, TEXT_HEIGHT, WHITE, BLUE);
        Rect(100, 135, 112, TEXT_HEIGHT, BLUE);
        Rect(80, 85, 160, TEXT_HEIGHT, BLUE); 
      }
      break;

    case 3: // Credits
      if (InsideMenu == 0)
      {
      pikachub_textbox(100, 135, 112, TEXT_HEIGHT, WHITE, BLUE);
      Rect(65, 110, 192, TEXT_HEIGHT, BLUE); 
      Rect(80, 85, 160, TEXT_HEIGHT, BLUE); 
      }
      break;
  }

  menu_select_option();

  // Select current menu option base on pushbutton 2
  switch (menu_option)
  {
    case 1:

      // Start Game
      if (menu_select)
      {
        delay(500);  
        P1_Information [0] = 160 - 30;
        P1_Information [1] = 120;
        P1_Information [2] = 0b000;
        P1_Information [3] = 0b00;
        P1_Information [4] = 3;
        P1_Information [5] = 0;
        
        P2_Information [0] = 160 + 30;
        P2_Information [1] = 120;
        P2_Information [2] = 0b000;
        P2_Information [3] = 0b00;
        P2_Information [4] = 3;
        P2_Information [5] = 0;

        Pika1_Information [0] = 40;
        Pika1_Information [1] = 66;
        Pika1_Information [2] = 1;
        Pika1_Information [3] = 10;

        Pika2_Information [0] = 120;
        Pika2_Information [1] = 180;
        Pika2_Information [2] = 0;
        Pika2_Information [3] = 10;

        Pika3_Information [0] = 180;
        Pika3_Information [1] = 85;
        Pika3_Information [2] = 1;
        Pika3_Information [3] = 10;

        Pika4_Information [0] = 215;
        Pika4_Information [1] = 30;
        Pika4_Information [2] = 1;
        Pika4_Information [3] = 10;

        Pika5_Information [0] = 40;
        Pika5_Information [1] = 90;
        Pika5_Information [2] = 0;
        Pika5_Information [3] = 10;

        Pika6_Information [0] = 40;
        Pika6_Information [1] = 55;
        Pika6_Information [2] = 0;
        Pika6_Information [3] = 10;

        EnemiesLeft = 6;
        GameWon = 0;
        GameLost = 0;
        AxisChange1 = 0;
        AxisChange2 = 0;
        
        FillRect(0, 0, 320, 240, 0xC748);
        LCD_Sprite (0, 0, 320, 240, Forest_Map, 1, 1, 0, 0);
        LCD_Sprite (P1_Information [0], P1_Information [1], 18, 31, entren_Green_Down_Walk, 6, 1, 0, 0);
        LCD_Sprite (P2_Information [0], P2_Information [1], 18, 31, entren_Green_Down_Walk, 6, 1, 0, 0);
        
        while (1)
        {
          
          Check_jugador_Movement(1, P1_Control_Valores, P1_Information, jugador1TiempoMov, AnimationP1Speed, jugador1AttackTime, P1AttackPosition, AttackP1Speed);
          Check_jugador_Movement(2, P2_Control_Valores, P2_Information, jugador1TiempoMov, AnimationP2Speed, jugador1AttackTime, P2AttackPosition, AttackP2Speed);
          
          //Move_Enemy_Vertical (Pika1_Information, PikaTiempoMov, AnimationEnemy1Speed);
          //Move_Enemy_Vertical (Pika2_Information, PikaTiempoMov, AnimationEnemy2Speed);
          AxisChange1++;
          AxisChange2++;
          if (AxisChange1 <= 100)
          {
          Move_Enemy_Horizontal (Pika1_Information, PikaTiempoMov,AnimationEnemy6Speed);
          Move_Enemy_Horizontal (Pika2_Information, PikaTiempoMov,AnimationEnemy6Speed);
          Move_Enemy_Horizontal (Pika3_Information, PikaTiempoMov,AnimationEnemy6Speed);        
          }
          else if (AxisChange1 > 100 && AxisChange1 <= 200)
          {
          

          Move_Enemy_Vertical (Pika1_Information, PikaTiempoMov, AnimationEnemy1Speed);
          FillRect(Pika1_Information [0], Pika1_Information [1] + 26, 26, 5, 0xC748);
          FillRect(Pika1_Information [0] + 25, Pika1_Information [1], 15, 26, 0xC748);
          
          Move_Enemy_Vertical (Pika2_Information, PikaTiempoMov, AnimationEnemy2Speed);
          FillRect(Pika2_Information [0], Pika2_Information [1] + 26, 26, 5, 0xC748);
          FillRect(Pika2_Information [0] + 25, Pika2_Information [1], 15, 26, 0xC748);
          
          Move_Enemy_Vertical (Pika3_Information, PikaTiempoMov, AnimationEnemy3Speed);
          FillRect(Pika3_Information [0], Pika3_Information [1] + 26, 26, 5, 0xC748);
          FillRect(Pika3_Information [0] + 25, Pika3_Information [1], 15, 26, 0xC748);
         
          }
          else 
          {
          AxisChange1 = 0;
          }
          
          if (AxisChange2 <= 50)
          {
          Move_Enemy_Horizontal (Pika4_Information, PikaTiempoMov,AnimationEnemy6Speed);
          Move_Enemy_Horizontal (Pika5_Information, PikaTiempoMov,AnimationEnemy6Speed);
          Move_Enemy_Horizontal (Pika6_Information, PikaTiempoMov,AnimationEnemy6Speed);
          }
          else if (AxisChange2 <= 100)
          {
            Move_Enemy_Vertical (Pika4_Information, PikaTiempoMov, AnimationEnemy4Speed);
          FillRect(Pika4_Information [0], Pika4_Information [1] + 26, 26, 5, 0xC748);
          FillRect(Pika4_Information [0] + 25, Pika4_Information [1], 15, 26, 0xC748);
          
          Move_Enemy_Horizontal (Pika5_Information, PikaTiempoMov,AnimationEnemy5Speed);
          FillRect(Pika5_Information [0], Pika5_Information [1] + 26, 26, 5, 0xC748);
          FillRect(Pika5_Information [0] + 25, Pika5_Information [1], 15, 26, 0xC748);

          Move_Enemy_Vertical (Pika6_Information, PikaTiempoMov,AnimationEnemy6Speed);
          FillRect(Pika6_Information [0], Pika6_Information [1] + 26, 26, 5, 0xC748);
          FillRect(Pika6_Information [0] + 25, Pika6_Information [1], 15, 26, 0xC748);
          }
          else
          {
            AxisChange2 = 0;
          }
          jugador_Detect_Collision (P1_Information, Pika1_Information,  AnimationP1Speed);
          jugador_Detect_Collision (P1_Information, Pika2_Information,  AnimationP1Speed);
          jugador_Detect_Collision (P1_Information, Pika3_Information,  AnimationP1Speed);
          jugador_Detect_Collision (P1_Information, Pika4_Information,  AnimationP1Speed);
          jugador_Detect_Collision (P1_Information, Pika5_Information,  AnimationP1Speed);
          jugador_Detect_Collision (P1_Information, Pika6_Information,  AnimationP1Speed);

          jugador_Detect_Collision (P2_Information, Pika1_Information,  AnimationP2Speed);
          jugador_Detect_Collision (P2_Information, Pika2_Information,  AnimationP2Speed);
          jugador_Detect_Collision (P2_Information, Pika3_Information,  AnimationP2Speed);
          jugador_Detect_Collision (P2_Information, Pika4_Information,  AnimationP2Speed);
          jugador_Detect_Collision (P2_Information, Pika5_Information,  AnimationP2Speed);
          jugador_Detect_Collision (P2_Information, Pika6_Information,  AnimationP2Speed);
          
          jugador_Attack_Collision (P1_Information, Pika1_Information);
          jugador_Attack_Collision (P1_Information, Pika2_Information);
          jugador_Attack_Collision (P1_Information, Pika3_Information);
          jugador_Attack_Collision (P1_Information, Pika4_Information);
          jugador_Attack_Collision (P1_Information, Pika5_Information);
          jugador_Attack_Collision (P1_Information, Pika6_Information);

          jugador_Attack_Collision (P2_Information, Pika1_Information);
          jugador_Attack_Collision (P2_Information, Pika2_Information);
          jugador_Attack_Collision (P2_Information, Pika3_Information);
          jugador_Attack_Collision (P2_Information, Pika4_Information);
          jugador_Attack_Collision (P2_Information, Pika5_Information);
          jugador_Attack_Collision (P2_Information, Pika6_Information);
          
          //debbuging
          //*********************************************
          if (P1_Information [4] <= 0 || P2_Information [4] <= 0)
          {
            GameLost = 1;
            FillRect(0,0,15,15, 0xF800);
            FillRect(0, 0, 320, 240, BLACK);
            LCD_Print("YOU LOST!", 80, 50, 2, WHITE, RED);
            LCD_Print("  all pokemons", 16, 110, 2, WHITE, BLACK);
            LCD_Print("scape in the forest", 16, 130, 2, WHITE, BLACK);
            lose++;
            myFile = SD.open("file.txt", FILE_WRITE);
            sprintf(Regis, "Vic: %d Der: %d", win, lose);  
            // if the file opened okay, write to it:
            if (myFile) {
                Serial.print("Writing to test.txt...");
                myFile.println(Regis);
                // close the file:
                myFile.close();
                Serial.println("done.");
              } else {
                // if the file didn't open, print an error:
                Serial.println("error opening test.txt");
              }           
           
            myFile = SD.open("file.txt");
            if (myFile) {
              Serial.println("file.txt:");
              while (myFile.available()) {
                Serial.write(lectura = myFile.read());
              }
          
              myFile.close();
            } else {
         
              Serial.println("error opening test.txt");
            }

            
            delay(5000);
            InsideMenu = 0;
            PrintedMainMenu = 0;
            break;
          }
        
          if (EnemiesLeft <= 0)
          {
            FillRect ((320-15), 0, 15, 15, 0x0000FF);
            FillRect(0, 0, 320, 240, BLACK);
            LCD_Print("LOS ATRAPASTE!", 80, 50, 2, WHITE, GREEN);
            LCD_Print("Han vuelto", 13, 110, 2, WHITE, BLACK);
            LCD_Print("Los pikachus", 13, 130, 2, WHITE, BLACK);
            LCD_Print("Bien hecho", 13, 150, 2, WHITE, BLACK);
            LCD_Print("entrenadores!", 13, 170, 2, WHITE, BLACK);
            win++;
        
              myFile = SD.open("file.txt", FILE_WRITE);
              sprintf(Regis, "Vic: %d Der: %d", win, lose);
              // if the file opened okay, write to it:
              if (myFile) {
                Serial.print("Writing to test.txt...");
                myFile.println(Regis);
                // close the file:
                myFile.close();
                Serial.println("done.");
              } else {
                // if the file didn't open, print an error:
                Serial.println("error opening test.txt");
              }
           
           //-----------------------------------------------------
            delay(10000);
            InsideMenu = 0;
            PrintedMainMenu = 0;
            GameWon = 1;
            break;
          }
          //*******************************************

          if (P1_Control_Valores [0] == 1)
          {
            InsideMenu = 0;
            PrintedMainMenu = 0;
            break;
          }
      }
        
      break;

    // Story and instructions
    case 2:

      if (menu_select)
      {
        print_story();
        print_instructions();

        while (1)
        {
            if (P1_Control_Valores [0] == 1)
            {
              InsideMenu = 0;
              PrintedMainMenu = 0;
              break;
            }
          }
        }     
      break;

    // Credits
    case 3:

      if (menu_select)
      {
        print_credits();

        delay(5000);
        InsideMenu = 0;
        PrintedMainMenu = 0;
        break;
      }
      
      break;
      }
  }
}

void Check_jugador_Movement (uint8_t Whichjugador, uint8_t Control_Valores [], int jugador_Information [], unsigned long jugadorTiempoMov, int AnimationSpeed, unsigned long jugadorAttackTime, int jugadorAttackPosition, int AttackSpeed )
{
  switch (Whichjugador)
  {
    case 1:
     if (Serial2.available() > 0) //Check if there is data available in the bus 
     {
        P1_Data = Serial2.read(); // Receive the jugador 1 Control data   
     }
  
      //Now, we process the jugador 1 data received:
      Control_Valores[0] = (P1_Data & 0b100000) >> 5;
      Control_Valores[1] = (P1_Data & 0b010000) >> 4;
      Control_Valores[2] = (P1_Data & 0b001111);
      break;
    
    case 2:
      if (Serial3.available() > 0) //Check if there is data available in the bus 
      {
        P2_Data = Serial3.read(); // Receive the jugador 1 Control data   
      }

      //Now, we process the jugador 1 data received:
      Control_Valores[0] = (P2_Data & 0b100000) >> 5;
      Control_Valores[1] = (P2_Data & 0b010000) >> 4;
      Control_Valores[2] = (P2_Data & 0b001111);
      break;
    
    default:
      break;
  }
  if (jugador_Information [4] > 0)
  {
    if (jugador_Information [5] == 0)     // Check if the jugador is in the walking state
    {
      switch (Control_Valores [2])     //We move the jugador according to joystick position
      {
        case 0b0001:    //RIGHT
  
          //Clean last sprite if last movement direction wasn't right        
          if (jugador_Information [2] != 0b0001)
          {
            FillRect(jugador_Information [0], jugador_Information [1], 24, 31, 0xC748);
          }
  
          
          jugador_Information [2] = 0b0001;    //Set current movement direction for the jugador
    
          if (jugador_Information [0] == (320 - 22 - 18))  // Check map boundaries collision
          {
            jugador_Information [0] = (320 - 22 - 18);     //If the jugador is in the boundary, don't continue moving
          }
    
          else if (micros() - jugadorTiempoMov >= 3000) // Move the jugador in the right direction 
          {     
            jugador_Information [0]++;
            jugadorTiempoMov = micros();
          }
          
          AnimationSpeed = ((jugador_Information [0])/15) % 6; // El módulo debe ser igual a la cantidad de columnas de su sprite. 
          LCD_Sprite(jugador_Information [0], jugador_Information [1], 24, 23, entren_Green_Lateral_Walk, 6, AnimationSpeed, 1, 0);
          V_line(jugador_Information [0] - 1, jugador_Information [1], 23, 0xC748);
          break;
    
        case 0b0100:    //LEFT
          
          //Clean last sprite if last movement direction wasn´t left
          if (jugador_Information [2] != 0b0100)   
          {
            FillRect(jugador_Information [0], jugador_Information [1], 24, 31, 0xC748);
          }
          
          jugador_Information [2] = 0b0100;    //Set current movement direction
    
          if (jugador_Information [0] == 30) // Check map boundaries collision
          {
            jugador_Information [0] = 30;    // If the jugador is on the boundary, don´t continue moving
          }
    
          else if (micros() - jugadorTiempoMov >= 3000) // Move the jugador in the left direction 
          {     
            jugador_Information [0]--;
            jugadorTiempoMov = micros();
          }
          
          AnimationSpeed = ((jugador_Information [0])/15) % 6; // El módulo debe ser igual a la cantidad de columnas de su sprite. 
          LCD_Sprite(jugador_Information [0], jugador_Information [1], 24, 23, entren_Green_Lateral_Walk, 6, AnimationSpeed, 0, 0);
          V_line(jugador_Information [0] + 24, jugador_Information [1], 23, 0xC748);
          break;
    
      case 0b0010:      // UP
        if (jugador_Information [2] != 0b0010)
        {
          FillRect(jugador_Information [0], jugador_Information [1], 24, 31, 0xC748);   //Clean last sprite if last movement direction wasn´t up
        }
        jugador_Information [2] = 0b0010;    //Set current movement direction
    
        if (jugador_Information [1] == 28)   // Check map boundaries collision
        {
          jugador_Information [1] = 28;      //If jugador is on the boundary, dont continue moving
        {     
          jugador_Information [1]++;
          jugadorTiempoMov = micros();
        }
        
        AnimationSpeed = ((jugador_Information[1])/15) %6;  // El módulo debe ser igual a la cantidad de columnas de su sprite
        LCD_Sprite (jugador_Information [0], jugador_Information [1], 18, 31, entren_Green_Down_Walk, 6, AnimationSpeed, 0, 0);
        H_line(jugador_Information [0], jugador_Information [1] - 1 , 18, 0xC748);
        break;
    
      default:
        break;
      }
  
      if (Control_Valores [1] == 0)
      {
        jugador_Information [5] = 0;
        jugador_Information [3] = 0b00;
      }
      else if(Control_Valores [1] == 1)
      {
        if ( jugador_Information [3] == 0b00)
        {
            if ((jugador_Information [0] >= 25 + 16) && (jugador_Information [0] + 49 <= 360 - 25 - 16))
            {
              if ((jugador_Information [1] >= 16 + 16) && (jugador_Information [1] + 49 <= 240 - 10 - 16))
              {
                jugador_Information [5] = 1;
                jugador_Information [3] = 0b01;     
              }
            }          
        }
        else {}
      }
          
    }
  
  
    else if (jugador_Information [5] == 1)
    {
      if (micros() - jugadorAttackTime >= 3000) // Attack Movement
      {   
          if (jugadorAttackPosition <= 32)
          { 
            if (Whichjugador == 1) P1AttackPosition++;
            else P2AttackPosition++;
            
            jugadorAttackTime = micros();
            AttackSpeed = ((jugadorAttackPosition)/2) %8;  // El módulo debe ser igual a la cantidad de columnas de su sprite
            LCD_Sprite (jugador_Information [0] - 16, jugador_Information [1] - 4 , 49, 49, Green_entren_Spin_Attack , 8, AttackSpeed, 0, 0);
          }
          else
          {
            jugador_Information [5] = 0;
            jugador_Information [2] = 0;
            FillRect(jugador_Information [0] - 16, jugador_Information [1] - 4, 49, 49, 0xC748);   //Clean last sprite if last movement direction wasn´t up
            LCD_Sprite (jugador_Information [0], jugador_Information [1], 18, 31, entren_Green_Down_Walk, 6, 1, 0, 0);
            
            if (Whichjugador == 1) P1AttackPosition = 0;
            else P2AttackPosition = 0;
          }
          
      }
    }
  }
};

void Move_Enemy_Vertical (int Pika_Information [], unsigned long PikaWalkingTime, int AnimationSpeed )
{
  if ( Pika_Information [3] > 0 )
  {
    if (Pika_Information [2] == 0)
    {
      if  (Pika_Information [1] >= 240 - 25 - 31)
      {
        Pika_Information [1] = 240 - 25 - 31;
        Pika_Information [2] = 1; 
      }
      else if (micros() - PikaWalkingTime >= 2000) // Move the jugador in the down direction 
      {     
      Pika_Information [1]++;
      PikaWalkingTime = micros();
      }
      
      AnimationSpeed = ((Pika_Information [1])/15) % 4; // El módulo debe ser igual a la cantidad de columnas de su sprite. 
      LCD_Sprite(Pika_Information [0], Pika_Information [1], 32, 25, PikaEnemy_Down_Walk, 4, AnimationSpeed, 0, 0);
      H_line(Pika_Information [0], Pika_Information [1] - 1, 32, 0xC748);
    }

    if (Pika_Information [2] == 1)
    {
      if  (Pika_Information [1] <= 28)
      {
        Pika_Information [1] = 28;
        Pika_Information [2] = 0; 
      }
      else if (micros() - PikaWalkingTime >= 2000) // Move the jugador in the up direction 
      {     
      Pika_Information [1]--;
      PikaWalkingTime = micros();
      }
      
      AnimationSpeed = ((Pika_Information [1])/15) % 4; // El módulo debe ser igual a la cantidad de columnas de su sprite. 
      LCD_Sprite(Pika_Information [0], Pika_Information [1], 31, 25, PikaEnemy_Up_Walk, 4, AnimationSpeed, 0, 0);
      H_line(Pika_Information [0], Pika_Information [1] + 25, 32, 0xC748);
    }    
  }
}

void Move_Enemy_Horizontal (int Pika_Information [], unsigned long PikaWalkingTime, int AnimationSpeed )
{
  if ( Pika_Information [3] > 0 )
  {
    if (Pika_Information [2] == 0)
    {
      if  (Pika_Information [0] >= 320 - 26 - 31)
      {
        Pika_Information [0] = 320 - 26 - 31;
        Pika_Information [2] = 1; 
      }
      else if (micros() - PikaWalkingTime >= 2000) //Move Pika to the left
      {     
      Pika_Information [0]++;
        }
        FillRect(Pika_Information [0], Pika_Information [1], 32, 25, 0xC748);   //Clean last jugador sprite if a collision is detected
        Pika_Information [1] = Pika_Information [1] + 45;
        if (Pika_Information [1] >= 240 - 25 - 21)
        {
          Pika_Information [1] = 240 - 25 - 31;
        }
      }
      else{};
    }

    else if ((Pika_Information [0] + 32) >= (jugador_Information [0] - 8) && (Pika_Information [0] + 32) <= (jugador_Information [0] + 44 +8 ))    //Detetect collision in the origin of the jugador bitmap (0,0)
    {
      if (Pika_Information [1] >= (jugador_Information [1] - 4) && Pika_Information [1] <= (jugador_Information [1] + 40))
      {
        Pika_Information [3] = Pika_Information [3] - 1;

        if (Pika_Information [3] == 0)
        {
            EnemiesLeft = EnemiesLeft - 1;
        }
          
        FillRect(Pika_Information [0], Pika_Information [1], 32, 25, 0xC748);   //Clean last jugador sprite if a collision is detected
        Pika_Information [1] = Pika_Information [1] + 45;
        if (Pika_Information [1] >= 240 - 25 - 21)
        {
          Pika_Information [1] = 240 - 25 - 31;
        }
      }
      else{};
    }

   if ((Pika_Information [0]) >= (jugador_Information [0] - 8) && (Pika_Information [0]) <= (jugador_Information [0] + 44 + 8))
    {
      if ((Pika_Information [1] + 25) >= (jugador_Information [1] - 4) && (Pika_Information [1] + 25) <= (jugador_Information [1] + 40))
      {
        Pika_Information [3] = Pika_Information [3] - 1;
        
        if (Pika_Information [3] == 0)
        {
            EnemiesLeft = EnemiesLeft - 1;
        }
          
        FillRect(Pika_Information [0], Pika_Information [1], 32, 25, 0xC748);   //Clean last jugador sprite if a collision is detected
        Pika_Information [1] = Pika_Information [1] - 45;
        if (Pika_Information [1] <= 28)
        {
          Pika_Information [1] = 28;
        }
        
      }
      else{};
    }
   
    if ((Pika_Information [0] + 32) >= (jugador_Information [0] - 8) && (Pika_Information [0] + 32) <= (jugador_Information [0] + 44 - 8))
    {
      if ((Pika_Information [1] + 25) >= (jugador_Information [1] - 4) && (Pika_Information [1] + 25) <= (jugador_Information [1] + 40))
      {
        Pika_Information [3] = Pika_Information [3] - 1;

        if (Pika_Information [3] == 0)
        {
            EnemiesLeft = EnemiesLeft - 1;
        }
          
        FillRect(Pika_Information [0], Pika_Information [1], 32, 25, 0xC748);   //Clean last jugador sprite if a collision is detected
        Pika_Information [1] = Pika_Information [1] - 45;
        if (Pika_Information [1] <= 28)
        {
          Pika_Information [1] = 28;
        }
        
      }
      else{};
    }
  }
}

void print_main_menu()
{ 
  if (PrintedMainMenu == 0)
  {
  // Background
  FillRect(0, 0, 320, 240, BLACK);
  FillRect(160-100, 120-40, 200, 80, BLUE);

  // Text
  LCD_Print("Pokemon Caught", 35, 40, 2, YELLOW, BLUE);
  LCD_Print("INICIAR", 80, 85, 2, WHITE, BLUE);
  LCD_Print("CONTROLS", 65, 110, 2, WHITE, BLUE);
  LCD_Print("INFO", 100, 135, 2, WHITE, BLUE);
  PrintedMainMenu = 1;
  }
  else {};
}

// Print instructions' screen
void print_instructions()
{
  // Background
  FillRect(0, 0, 320, 240, BLACK);

  // Text
  LCD_Print("Controles:", 16, 16, 2, WHITE, BLACK);
  LCD_Print("Joystick: Move", 64, 48, 2, WHITE, BLACK);
  LCD_Print("A: Atrapa", 64, 80, 2, WHITE, BLACK);
  LCD_Print("(B) Regreso menu", 16, 175, 2, WHITE, BLUE);
  delay(5000);
}

void print_story()
{
  // Background
  FillRect(0, 0, 320, 240, BLACK);

  // Text
  LCD_Print("Los pikachus del", 16, 16, 2, WHITE, BLACK);
  LCD_Print("proferos Oak", 16, 48, 2, WHITE, BLACK);
  LCD_Print("Se han escapado", 16, 80, 2, WHITE, BLACK);
  LCD_Print("del laboratorio,", 16, 112, 2, WHITE, BLACK);
  LCD_Print("perdiendose en", 16, 144, 2, WHITE, BLACK);
  LCD_Print("el bosque", 16, 180, 2, WHITE, BLACK);

  delay(5000);

  FillRect(0, 0, 320, 240, BLACK);
  
  LCD_Print("Dos entrenadores", 16, 16, 2, WHITE, BLACK);
  LCD_Print("valientes deben", 16, 48, 2, WHITE, BLACK);
  LCD_Print("atraparlos para", 16, 80, 2, WHITE, BLACK);
  LCD_Print("llevarlos de vuelta", 16, 112, 2, WHITE, BLACK);
  LCD_Print("junto al profesor", 16, 144, 2, WHITE, BLACK);

  delay(5000);
}

// Print credits' screen
void print_credits()
{
  // Background
  FillRect(0, 0, 320, 240, BLACK);


  // Text
//  sprintf(Regis1, "d", lectura);
//  printf(Regis1);
  LCD_Print("Creado por:", 16, 16, 2, WHITE, BLACK);
  LCD_Print("Jose y Sergio", 64, 48, 2, WHITE, BLACK);
  LCD_Print("Lugar:", 16, 80, 2, WHITE, BLACK);
  LCD_Print("Universidad del Valle", 64, 112, 2, WHITE, BLACK);
  LCD_Print("de Guatemala", 64, 144, 2, WHITE, BLACK);
  LCD_Print("Registros: ", 16, 180, 2, WHITE, BLACK);
  LCD_Print(Regis, 64, 212, 2, WHITE, BLACK);
}

// "Bentren animation" for the text in the menu's options.
void bentren_textbox(int x, int y, int width, int height, int color_on, int color_off)
{ 
  if (millis() - BentrenBoxTimeON >= 200)
  {
  Rect(x, y, width, height, color_on);
  BentrenBoxTimeON = millis();
  }
  if (millis() - BentrenBoxTimeOFF >= 400)
  {
  Rect(x, y, width, height, color_off);
  BentrenBoxTimeOFF = millis();
  } 
}

void print_map()
{
  FillRect(0, 0, 320, 240, GREEN);
  FillRect(0, 0, 320, TILE_SIZE, WHITE);
  FillRect(0, 224, 320, TILE_SIZE, WHITE);
  FillRect(0, 0, TILE_SIZE, 240, WHITE);
  FillRect(304, 0, TILE_SIZE, 240, WHITE);
}

int check_borders_collision(int sprite_x, int sprite_y)
{
 
  if (sprite_x < LEFT_BORDER)
  {
    return 1;
  }

  else if (sprite_x > RIGHT_BORDER)
  {
    return 1;
  }

  else if (sprite_y < BOTTOM_BORDER)
  {
    return 2;
  }

  else if (sprite_y > TOP_BORDER)
  {
    return 2;
  }

  else
  {
    return 0;
  }
}

void menu_navigate()
{
  if (millis() - MenuDebounceDelay >= 500)
  {
    if (P1_Control_Valores [1] == 1)
    {
      menu_option++;
      MenuDebounceDelay = millis();
  
      if (menu_option > 3)
      {
        menu_option = 1;
      }
    }
  }
}

void menu_select_option()
{
  if (millis() - SelectOptionDebounceDelay >= 10)
  {
    if (P1_Control_Valores [0] == 1 && P1_Information [3] == 0)
    {
      InsideMenu = 1;
      menu_select = 1;
      SelectOptionDebounceDelay = millis();
    }
  
    else
    {
      menu_select = 0;
      SelectOptionDebounceDelay = millis();
    }
  }
}
