//Phoenix Game Development
//27/9/2015


//This function originally obtained from here:
//http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
//Rewritten for processing by Phoenix Game Development

color hsv2rgb(int huein)
{
  //println(huein);
  float h = abs(255-huein);
  float s = 128.0;
  float v = 128.0;

  float r = 0.0;
  float g = 0.0;
  float b = 0.0;

  float      hh, p, q, t, ff;
  int        i;
  color         out;

  if (s <= 0.0) {       // < is bogus, just shuts up warnings
    r = v;
    g = v;
    b = v;

    out = color(r, g, b);

    return out;
  }
  hh = h;
  if (hh >= 360.0) hh = 0.0;
  hh /= 60.0;
  i = (int)hh;
  ff = hh - i;
  p = v * (1.0 - s);
  q = v * (1.0 - (s * ff));
  t = v * (1.0 - (s * (1.0 - ff)));

  switch(i) {
  case 0:
    print("0");
    print(" ");
    print(r);
    print(" ");
    print(g);
    print(" ");
    print(b);
    print(" ");
    print(h);
    print(" ");
    print(huein);

    println("");
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    println("1");
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    println("2");
    r = p;
    g = v;
    b = t;
    break;

  case 3:
    println("3");
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    println("4");
    r = t;
    g = p;
    b = v;
    break;
  case 5:
    println("5");
  default:
    r = v;
    g = p;
    b = q;
    break;
  }
  out = color(r, g, b);
  return out;
}

int incamount = 4;

int[][] rawarray = new int[80][60];
int[][] interpolatedarray = new int[80*2][60*2];


void renderrawimage() {

      int xpos = 0;
    int ypos = 0;
    
  for (int i = 0; i < 80; i++) {

    for (int j = 0; j < 60; j++) {

      int col = rawarray[i][j];
      
     color c =  hsv2rgb(col);
            
      noStroke(); //prevents visible borders on squares

      fill(c);
      // println(col);

      rect(xpos, ypos, 4, 4);

      xpos+=incamount;

      if (xpos >=(80*incamount)) {
        xpos = 0;
        ypos+=incamount;
      }

      if (ypos >= (60*incamount))
        ypos = 0;
    }
  }
}

void populateinterpolatedarray(){
  
  for (int i = 0; i < 80*2; i+=2) {

    for (int j = 0; j < 60*2; j+=2) {

      int col = rawarray[i][j];
      
      interpolatedarray[i][j] = col;
       interpolatedarray[i+1][j+1] = 255;
      
      
     color c =  hsv2rgb(col);

    }
}
}

  void renderinterpolatedimage() {
     //first, populate the interpolated array:
        populateinterpolatedarray();
        
    int xpos = 0;
    int ypos = 0;
    
 
    
  for (int i = 0; i < 80*2; i++) {

    for (int j = 0; j < 60*2; j++) {

      int col = interpolatedarray[i][j];
      
     color c =  hsv2rgb(col);
            
      noStroke(); //prevents visible borders on squares

      fill(c);
      // println(col);

      rect(xpos, ypos, 4, 4);

      xpos+=incamount;

      if (xpos >=(80*incamount)) {
        xpos = 0;
        ypos+=incamount;
      }

      if (ypos >= (60*incamount))
        ypos = 0;
    }
  }
     
  }

  void setup() {
    size(640, 480);

    String lines[] = loadStrings("output.txt"); 

    String image = "";
    for (int i = 0; i < lines.length; i++) {
      //println(lines[i]);
      image = image + lines[i];
    }

int xpos = 0;
int ypos = 0;

    int count = 0;

    int[] dataasint = int(split(image, ' '));

  for (int i = 0; i < 80; i++) {

    for (int j = 0; j < 60; j++) {
      
      int col = dataasint[count];
      
      rawarray[i][j] = col;
      
      count++;
      
    }
    
  }

//renderrawimage();
renderinterpolatedimage();
  //  for (int i : dataasint) {   
      // }
      
    println("COUNT: ");

    println(count);

    println();
  }