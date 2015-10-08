//Phoenix Game Development
//27/9/2015


//This function originally obtained from here:
//http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
//Rewritten for processing by Phoenix Game Development

int R=0, G=0, B=0;
void hsvl2rgb(float H, float L)
{
  println(H);
  // H=0.28;
  int var_i;
  float S=1, V, lightness, var_1, var_2, var_3, var_h, var_r, var_g, var_b;

  V = L * 2;                         // For the "darkness" of the LED
  if ( V > 1 ) V = 1;

  if ( S == 0 )                      //HSV values = 0 ÷ 1
  {
    R = Math.round(V * 255);
    G = Math.round(V * 255);
    B = Math.round(V * 255);
  } else
  {
    var_h = H * 6;
    if ( var_h == 6 ) var_h = 0;  //H must be < 1
    var_i = int( var_h ) ;            //Or ... var_i = floor( var_h )
    var_1 = V * ( 1 - S );
    var_2 = V * ( 1 - S * ( var_h - var_i ) );
    var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

    if ( var_i == 0 ) {
      var_r = V     ;
      var_g = var_3 ;
      var_b = var_1 ;
    } else if ( var_i == 1 ) {
      var_r = var_2 ;
      var_g = V     ;
      var_b = var_1 ;
    } else if ( var_i == 2 ) {
      var_r = var_1 ;
      var_g = V     ;
      var_b = var_3 ;
    } else if ( var_i == 3 ) {
      var_r = var_1 ;
      var_g = var_2 ;
      var_b = V     ;
    } else if ( var_i == 4 ) {
      var_r = var_3 ;
      var_g = var_1 ;
      var_b = V     ;
    } else {
      var_r = V     ;
      var_g = var_1 ;
      var_b = var_2 ;
    }

    if ( L > 0.5 )         // Adjusting the Lightness (whiteness)
    {
      lightness = ( L - 0.5 ) / 0.5;
      var_r += ( lightness * ( 1 - var_r ) );
      var_g += ( lightness * ( 1 - var_g ) );
      var_b += ( lightness * ( 1 - var_b ) );
    }

    R = Math.round((1-var_r) * 255);     // RGB results = 0 ÷ 255. Reversed for common anode RGB LED's
    G = Math.round((1-var_g) * 255);
    B = Math.round((1-var_b) * 255);
  }
}


float[][] rawarray = new float[80][60];
float[][] interpolatedarray = new float[80*2][60*2];

int incamount2 = 8;
void renderrawimage() {

  int xpos = 0;
  int ypos = 0;

  for (int i = 0; i < 80; i++) {

    for (int j = 0; j < 60; j++) {

      float col = rawarray[i][j];

      //void hsvl2rgb(float H, float L)
      hsvl2rgb(col, 0.5);//hsv2rgbl(col);

      print(R);
      print(" ");
      print(G);
      print(" ");
      print(B);
      print(" ");
      println();

      color c = color(R, G, B);
      noStroke(); //prevents visible borders on squares

      fill(c);
      // println(col);

      rect(xpos, ypos, incamount2, incamount2);

      xpos+=incamount2;

      if (xpos >=(80*incamount2)) {
        xpos = 0;
        ypos+=incamount2;
      }

      if (ypos >= (60*incamount2))
        ypos = 0;
    }
  }
}

void populateinterpolatedarray() {

  for (int i = 0; i < 80; i++) {

    print("1: ");
    for (int j = 0; j < 60; j++) {

      float col;

      int ix = 0;
      int iy = 0;

      if (i < 80 && j < 60) {
        ix = i;
        iy = j;
        col = rawarray[ix][iy]; //Get "Raw" value
      } else {
        col = 0;
      }
      col = rawarray[i][j]; //Get 

      // interpolatedarray[i][j] = col; //set raw value (this does not change)

      //Interpolate:
      //scale new points to correct range:
      int newx = i*1;
      int newy = j*1;

      interpolatedarray[newx][newy] = col;
      //  interpolatedarray[newx][newy+1] = col;
      //  interpolatedarray[newx+1][newy] = col;
      // interpolatedarray[newx+1][newy+1] = col;

      print(col);
      print(" ");

      // color c =  hsv2rgb(col);
    }
    println("1*");
  }
}

int incamount = 8;
void renderinterpolatedimage() {
  
  int xpos = 0;
  int ypos = 0;

  // int resx = 80*2;
  //  int resy = 60*2;

  for (int i = 0; i < 80; i++) {
    print("2: ");
    for (int j = 0; j < 60; j++) {

      noStroke(); //prevents visible borders on squares
      //stroke(0,0,0);
      
      //Grab neighbouring values:
      float value1 = rawarray[i][j];  

      int ix = i;
      int iy = j;

      if (ix >=79)
        ix = 78;

      if (iy >=59)
        iy = 58;

      float value2 = rawarray[ix][iy+1];    
      float value3 = rawarray[ix+1][iy];   
      float value4 = rawarray[ix+1][iy+1];

      float ivalue1 = lerp(value1, value2, 0.5);//(value1+value2)/2;
      //float ivalue2 = lerp(value1, value3, 0.5);//(value1+value3)/2;
      //float ivalue3 = lerp(value1, value4, 0.5);//(value1+value4)/2;

      xpos+=incamount;

      if (xpos >=(80*incamount)) {
        xpos = 0;
        ypos+=incamount;
      }

      if (ypos >= (60*incamount)) {
        ypos = 0;
      }

      color c;
      //color c = Color.HSBtoRGB(h, s, b);
      hsvl2rgb(value1, 0.5);
      c = color(R, G, B);    
      fill(c);
      rect(xpos, ypos, incamount, incamount);

      hsvl2rgb(ivalue1, 0.5);
      c = color(R, G, B);
      fill(c);
      rect(xpos+4, ypos, incamount, incamount);

      hsvl2rgb(ivalue1, 0.5);
      c = color(R, G, B);
      fill(c);
      rect(xpos, ypos+4, incamount, incamount);

      hsvl2rgb(ivalue1, 0.5);
      c = color(R, G, B);
      fill(c);
      rect(xpos+4, ypos+4, incamount, incamount);
    }

    println("2*");
  }
}

void setup() {
  size(1024, 768);

  String lines[] = loadStrings("output0.lrf"); 

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

      if (count < dataasint.length) {
        int col = dataasint[count];

        float ret = (((col - 0) * (1.0 - 0.0)) / (255 - 0)) + 0.0;

        //   print(col);
        //   print(" ");
        
        rawarray[i][j] = ret;

        count++;
      }
    }
    println("");
  }

  // renderrawimage();
  renderinterpolatedimage();

  println("COUNT: ");
  println(count);
  println();
}