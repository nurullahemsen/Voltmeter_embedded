/**
  *TM4C123 serisi development board Kullanılarak geliştirilmiş
  *dijital Voltmetre nin bilgisayar üzerinden oluşturulunan
  *izleme (monitoring) arayüzü.
  *
  *Girdi olarak 
  *  '$' akabinde iki byte ve sonunda '%' karakterleri serisi yada re: '$'(byte)(byte)'%'
  *  '#' akabinde bir karakter ('+' veya '-') ve sonunda '%' karakterleri serisi re:'#'('+' | '-')'%'
  *   RE: (('$'(byte)(byte)) | '#'('+' | '-'))'%'
  *beklenir aksi takdirde hiçbirşey yapılmaz
  *
  *Program çalıştırıldığında aynı anda birden fazla cihaz serial COM portunu kullanıyorsa
  *kodda spesifik olarak cihazımızın COM port'u belirtilmelidir
  *örneğin: port0 = new Serial(this, "COM11", 9600);
  *
  *05160000657 Muhammed Nurullah EMSEN
  *05160000784 Elifnaz ÖKLÜ
  *05160000283 Oğuzhan KATI
  *05150000628 Ayşenur BAĞ
  *
  *2.01.2020
  *
**/




import processing.serial.*;

Serial port0;
ArrayList<Vpoint> pList = new ArrayList<Vpoint>();
int x;
int y = 0;
int v256 = 0;
int vCarry = 0;
int shiftAmount = +3;

void setup() {
  //size(1900, 1024);
  fullScreen();
  x = width;
  y = height/2;
  port0 = new Serial(this, Serial.list()[0], 9600); //Serial port tanımlanırs
}

void draw(){
  drawBackground();
  stroke(255,0,0);
  strokeWeight(8);
  noFill();
  beginShape();
  if(pList.size()>2000) pList.remove(0); // en fazla en yeni 2000 nokta bellekte tutulsun
  for(Vpoint px : pList){ //Her frame de noktaları ölçeğe göre soldan sağa kaydır
    px.shift(shiftAmount);
    if(px.x>0)vertex(px.x,px.y); //ekran dışıına taşan noktaları çizdirmeye çalışma
  }
  endShape();
  while (port0.available() > 0) { // Eğer serial buffer'ında karakter varsa //<>//
    //y = port0.read();
    int r = port0.read();
    if(r =='#'){ //Karakter # ise monitor ölçekleme yapacak
      //pList.clear();
      r = port0.read(); // ölçeklemenin arttırılacağı veya eksiltileceğini gösterecek karakter
      if(port0.read() != '%')continue; //eğer karakter silsilesi sonunda % alınmadıysa hiçbirşey yapılmaz
      if(r == '+'){ // ölçekleme arttırılacak
        shiftAmount++;
        if(shiftAmount > 9){ //değer 9 u geçmesin
          shiftAmount = 9;
          continue;
        }
        shiftPoints(-1); //halihazırdaki tüm noktalar yeni ölçeğe göre yeniden düzenlenir
      }
      else if(r == '-'){ // ölçekleme azaltılacak
        shiftAmount--;
        if(shiftAmount < 1){ //değer 1 den küçük olmasın
          shiftAmount = 1;
          continue;
        }
        shiftPoints(+1); //halihazırdaki tüm noktalar yeni ölçeğe göre yeniden düzenlenir
      }

    }
    else if(r =='$'){ //Karakter $ ise monitor noktayı ekrana çizdirecek
      v256 = port0.read(); //Most significant 4 bit (256 lar)
      vCarry = port0.read(); //least significant 8 bit (256 kalanları)
      if(port0.read()=='%')y = v256*256+vCarry; //Son okunankarakter % değilse değişiklik yapma
      //if(y > 1024) continue;
      println(y);
    }
    
  }
  Vpoint p = new Vpoint(x-25,1024-y/4+55); //Yeni nokta tanımlanır ve listeye eklenir.
  p.drawPoint();
  pList.add(p);
}
/*
  Her ölçekleme değişiminde noktaları yeni olçeklerine
  göre yeniden konumlandıran fonksiyon
*/
void shiftPoints(int amount){
  int total = 0;
  for(int index = pList.size()-1; index >= 0; index--){
    total = total + amount;
    pList.get(index).shift(-total);
  }

}
/*
  Her frame de arka plandaki çizgileri ölçeklemeye uygun olarak
  çizilmesini sağlayan fonksiyon.
*/
void drawBackground(){
  background(0);
  strokeWeight(1);
  stroke(0,60,0);
  // yatay çizigiler
  for(int i = 1; i<7; i++){
    float y0 = map(i*0.5,0.0,3.3,0,1024);
    line(0, y0, width, y0);
  }
  //3.3 V belirten en üstteki çizgi
  line(0, height-1024, width, height-1024);
  
  //Dikey çizgiler; ölçeklemeye göre değişkenler
  stroke(0,40,0);
  int lineWidth = shiftAmount*50;
  for(int i= 1; i <= 1920/(lineWidth); i++){
    int constantX = width - i*shiftAmount*50;
    line(constantX, height-1024, constantX, height);
  }
}
