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


void setup() {
  size(640, 480);

  String lines[] = loadStrings("output.txt"); 

  String image = "";
  for (int i = 0; i < lines.length; i++) {
    //println(lines[i]);
    image = image + lines[i];
  }


  int count = 0;

  int xpos = 0;
  int ypos = 0;

  int incamount = 4;

  int[] dataasint = int(split(image, ' '));

  for (int i : dataasint) {

    int col = dataasint[count];//image.charAt(i);
    count++;
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
  println("COUNT: ");

  println(count);

  println();
}