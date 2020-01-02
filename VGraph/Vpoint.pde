/*
  Voltmetre Monitor programında kullanılacak nokta sınıfı
*/
public class Vpoint{
  public int x;
  public int y;
  
  Vpoint(int x, int y){
    this.x = x;
    this.y = y;
  }
  
  void shift(int amount){
    this.x -= amount;
  }
  
  public void drawPoint(){
    point(this.x,this.y);
  }

}
