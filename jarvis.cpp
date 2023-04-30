//PROGETTO DSP FILIPPO GIOVAGNOLI

/*
COMANDI:
1 dito: finestra (ingrandisco o rimpicciolisco)
2 dita: volume (alzo o abbasso)
3 dita: scroll (verso l'alto o verso il basso)
4 dita: help (mostra comandi)
5 dita: niente
*/

//Memo: modifica il file CMakeLists.txt con il nome di questo file!
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cmath>
#include <sstream>
using namespace cv;
using namespace std;

int valore_soglia;
int tipo_soglia;
int max_soglia;
int sizeframe=640;//frame quadrato per semplicità
void on_trackbar(int,void*){}

int main(int argc, char **argv){
/////////////////////VARIABILI DI CALIBRAZIONE//////////////////////////////
int lunghezzadito=70; //soglia per il riconoscimento del dito: 100 mano6 130 mano2
int lunghezzaditomax=0;
float sens_chiusa=0.2; //sensibilità chiusura pugno
float sens_dito=0.25; //0.2
valore_soglia=100; //fondo scuro: 100 fondo chiaro: 170
max_soglia=255;
tipo_soglia = 0;// fondo scuro: 0 fondo chiaro: 1
namedWindow("Controllo soglia",1);
createTrackbar("Soglia","Controllo soglia",&valore_soglia,max_soglia,on_trackbar);
///////////////////VARIABILI///////////////////////////////////////////////
Mat originale, originale_gray, binaria;
vector<vector<Point>> contorni;
vector<Point> contorno_mano;
Point centro_prec(0,0);
Point centro;
Point indice;
Point indice_prec;
int area_prec=0;
int ditacontate_prec=0;
int i=0;//conta i frame
int conteggio=0;//conta i pixel bianchi per capire se la soglia è giusta
int i_salv=0;
VideoCapture cap;
/////////////////////SETUP///////////////////////////////////////////////////
if (strcmp(argv[1],"webcam")==0){
 cap.open(0);//numero della webcam
 if(!cap.isOpened()) {printf("Impossibile accedere alla webcam\n");}
}
else{
 cap.open(argv[1]); //nome video 
 if(!cap.isOpened()) {printf("Impossibile aprire il video\n");}
}
///////////////RETTANGOLI///////////////////////////////////////////////////
int ottavo=sizeframe/8;//640 dimensione del frame quadrato
Rect botton1(5*ottavo,ottavo,1.5*ottavo, 4*ottavo); //DOWN CONTROL
Rect botton2(ottavo,ottavo,1.5*ottavo,4*ottavo); //UP CONTROL
Rect botton3(2.5*ottavo+1,ottavo,2.5*ottavo-3,1.5*ottavo);//Next control
string message="";
/////////////////LOOP E SOGLIA/////////////////////////////////////////////////
while(1){
   cap >> originale; //no  = !!!
   if (originale.empty()) {break;}      
   resize(originale,originale, Size(sizeframe,sizeframe)); //utile a ridurre la qualità
   if (strcmp(argv[1],"webcam")==0){
     cvtColor(originale,originale,COLOR_BGR2RGB);//la pi camera scambia l'ordine dei colori
   }
   GaussianBlur(originale,originale_gray, Size(7,7),14); //utile a ridurre la qualità
   cvtColor( originale_gray, originale_gray, COLOR_BGR2GRAY ); // Converto l'immagine in grigio
   threshold( originale_gray, binaria, valore_soglia, 255, tipo_soglia );
/////////////////////CONTROLLO SFONDO//////////////////////////////////////////
   if (i==3){ //controllo che la soglia sia giusta verificando i bordi
     for (int i=0; i<640; i++){
      if ((binaria.at<uchar>(0,i) == 255 && binaria.at<uchar>(639,i) == 255)||(binaria.at<uchar>(i,0) == 255 && binaria.at<uchar>(i,639) == 255))
      {conteggio++;}
     }
   if (conteggio>300){tipo_soglia=1;conteggio=0;}
   else {tipo_soglia=0;conteggio=0;}
   }
//////////////////////CONTORNI////////////////////////////////////////////////
   findContours(binaria, contorni, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
   int indice_contorno_grosso=-1;
   int area_contorno_grosso=0;
   for (int i=0;i<contorni.size(); i++){
	int area=contourArea(contorni[i]);
      if (area>area_contorno_grosso){
        indice_contorno_grosso=i;
        area_contorno_grosso=area;
      }
   }
   if (indice_contorno_grosso>=0){
    Rect rettangolo=boundingRect(contorni[indice_contorno_grosso]);
    rectangle(originale,rettangolo,Scalar(100,100,0),2);
    //centro=(rettangolo.br()+rettangolo.tl())*0.5; //cerco il centro del rettangolo
    //circle(originale, centro,3,Scalar(0,0,255)); //lo disegno
    contorno_mano=contorni[indice_contorno_grosso];
   } 
//////////////////////DITA//////////////////////////////////////////////////
  lunghezzadito=sqrt(area_contorno_grosso/30)*2.3;
  if (lunghezzadito>(1+sens_dito)*lunghezzaditomax){lunghezzaditomax=lunghezzadito;}
  int ditacontate=0; 
  if (!contorno_mano.empty()){
     vector<int> hull; //trovo punti di convessità nel contorno
     convexHull(contorno_mano, hull, true);
     vector<Vec4i>  difetti; //difetti di convessità
     convexityDefects(contorno_mano,hull,difetti);
       for (size_t i =0; i<difetti.size();i++){
       Point p1=contorno_mano[difetti[i][0]];
       Point p2=contorno_mano[difetti[i][1]];
       Point p3=contorno_mano[difetti[i][2]];
       float profondita= difetti[i][3]/256.0;
       float distanza = norm(p2-p3); //se la distanza è superiore a una soglia (lunghezza dito) p2-p3
          if ((profondita>(sens_dito*lunghezzaditomax))&&(distanza>lunghezzaditomax)&&(p2.x>20)&&(p2.x<620)){
              indice=p2;
	      ditacontate++;
	      line(originale,p2,p3, Scalar(20, 255, 20), 2); //centro
          }
      }
   }
/////////////////////TESTO IN ALTO/////////////////////////////////////////////
   string text ="";
   if(ditacontate==0){text="No fingers detected";}
   else if (ditacontate==1){text="Window";}
   else if (ditacontate==2){text="Volume";}
   else if (ditacontate==3){text="Scroll";}
   else if (ditacontate==4){text="Help";}
   else if (ditacontate==5){text="High five!";}
   else if (ditacontate>5){text="Too many fingers!";}
   putText(originale,text,Point(ottavo/3,ottavo/3), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
   circle(originale,indice,7,Scalar(255,255,0)); //disegno il dito indice
/////////////////////////TESTO ALTERNATIVO////////////////////////////////////
putText(originale, message,Point(20,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
message="";
/////////////////////////AREA///////////////////////////////////////////////
   if ((i%5==0)&&(i>50)){ //prendo un frame ogni 5 non partendo subito
     if ((area_prec>(1+sens_chiusa)*area_contorno_grosso)&&(area_contorno_grosso!=0)&&(ditacontate<3)&&(ditacontate_prec==5))
     {  printf("KILL!\n");
        int a=system("xdotool getwindowfocus windowkill");
      }//MANO CHIUSA
     else if ((area_prec<(1-sens_chiusa)*area_contorno_grosso)&&(i_salv!=0)&&(ditacontate>4))
     {}//MANO APERTA
   area_prec=area_contorno_grosso;
   centro_prec=centro;
   ditacontate_prec=ditacontate;
   }
   i++; //conto i frame

/////////INTERFACCIA E CONTROLLI//////////////////////////////////////////////

if (ditacontate==1){//Window 
arrowedLine(originale,Point(2.5*ottavo+20,ottavo+15),Point(4.5*ottavo,ottavo+15),Scalar(255,0,0),2); 
arrowedLine(originale,Point(((5*ottavo)+20),ottavo+10),Point(((5*ottavo)+20),4*ottavo),Scalar(0,0,255),2); 
arrowedLine(originale,Point(ottavo+20,4*ottavo),Point(ottavo+20,ottavo+10),Scalar(0,255,0),2);
putText(originale,"Max",Point(ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);   
putText(originale,"Min",Point(5*ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
putText(originale,"Next",Point(3*ottavo,(ottavo-5)), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
rectangle(originale,botton1,Scalar(0,0,255),1);
rectangle(originale,botton2,Scalar(0,255,0),1);
rectangle(originale,botton3,Scalar(255,0,0),1);
   if (i%5==0){
     if ((indice_prec.x+0.1*sizeframe)<indice.x){
     int a=system("xdotool key alt+Tab "); 
     putText(originale,"Next",Point((3*ottavo),(ottavo-5)), FONT_HERSHEY_SIMPLEX,1,Scalar(255,0,0),2);
     }
   if (botton1.contains(indice)){
     if((indice_prec.y+0.1*sizeframe)<indice.y){
     int a=system("xdotool windowsize $(xdotool getactivewindow) 10% 10%");
     putText(originale,"Min",Point(5*ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,255),2);
     }
   }
  else if (botton2.contains(indice)){
     if(indice_prec.y>(indice.y+0.1*sizeframe)){
     int a=system("xdotool windowsize $(xdotool getactivewindow) 50% 50%");
     putText(originale,"Max",Point(ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(0,255,0),2);
     }
   }
   indice_prec=indice;
   }
}


else if (ditacontate==2){//Volume
arrowedLine(originale,Point((5*ottavo+20),ottavo+10),Point((5*ottavo)+20,4*ottavo),Scalar(0,0,255),2); 
arrowedLine(originale,Point(ottavo+20,4*ottavo),Point(ottavo+20,ottavo+10),Scalar(0,255,0),2);
putText(originale,"Up",Point(ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);   
putText(originale,"Down",Point(5*ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);   
rectangle(originale,botton1,Scalar(0,0,255),1);
rectangle(originale,botton2,Scalar(0,255,0),1);
   if (i%5==0){ 
    if(botton1.contains(indice)){
     if((indice_prec.y+0.1*sizeframe)<indice.y){
     int a=system("amixer -q sset Master 10%-");
     putText(originale,"Down",Point(5*ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,255),2);
     }
    }
    else if (botton2.contains(indice)){
     if(indice_prec.y>(indice.y+0.1*sizeframe)){
     int a=system("amixer -q sset Master 10%+");
     putText(originale,"Up",Point(ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(0,255,0),2);
    }
   }
   indice_prec=indice;
   }
}


else if (ditacontate==3){//Scroll
arrowedLine(originale,Point((5*ottavo+20),ottavo+10),Point((5*ottavo)+20,4*ottavo),Scalar(0,0,255),2); 
arrowedLine(originale,Point(ottavo+20,4*ottavo),Point(ottavo+20,ottavo+10),Scalar(0,255,0),2);
putText(originale,"Up",Point(ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);   
putText(originale,"Down",Point(5*ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);   
rectangle(originale,botton1,Scalar(0,0,255),1);
rectangle(originale,botton2,Scalar(0,255,0),1);
   if (i%5==0){ 
    if(botton1.contains(indice)){
     if((indice_prec.y+0.1*sizeframe)<indice.y){
     int a=system("xdotool click 5 click 5");
     putText(originale,"Down",Point(5*ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,255),2);
     }
    }
    else if (botton2.contains(indice)){
     if(indice_prec.y>(indice.y+0.1*sizeframe)){
     int a=system("xdotool click 4 click 4");
     putText(originale,"Up",Point(ottavo,ottavo-10), FONT_HERSHEY_SIMPLEX,1,Scalar(0,255,0),2);
    }
   }
   indice_prec=indice;
   }
}


else if(ditacontate==4){ //help
putText(originale,"1 finger: Window control",Point(0,2*ottavo), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
putText(originale,"2 fingers: Volume control",Point(0,3*ottavo), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
putText(originale,"3 fingers: Scroll",Point(0,4*ottavo), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
putText(originale,"4 fingers: Help",Point(0,5*ottavo), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
putText(originale,"5 fingers: Nothing",Point(0,6*ottavo), FONT_HERSHEY_SIMPLEX,1,Scalar(255,255,255),2);
}

//////////////////OUTPUT//////////////////////////////////////////////////////
imshow("Mano Riconosciuta", originale);
imshow("Controllo soglia", binaria);
/////////////////FINE////////////////////////////////////////////////////////
   char c =(char)waitKey(25);
   if (c==27){break;}//esc per uscire
   else if (c==104){ // "h" per avere un manuale
   printf("\n\n");
   int a=system("cat manual.txt");
   } 
   else if (c==32){ //space per stoppare il programma
    while (1){
     sleep(2);
     char g=(char)waitKey(25);
     if (g==32){break;}//space per riprendere il programma
     else if(g==27){break;}
    }
  }
} //esco dal while iniziale
cap.release();
destroyAllWindows();
return 0;
}
