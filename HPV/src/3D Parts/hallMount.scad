$fn=20;
hallx=15.6;
hally=23.3;
hallz=8;
zipw=5;
ziph=2;
wiredia=5;
margin=zipw+2;
tubedia=13.3;
difference()
 {
cube([hallx+margin,hally+margin*3+tubedia/2,hallz+margin]);
translate([margin/2,margin*3+tubedia/2,margin]) cube([hallx,hally,hallz]);
translate([0,tubedia/2+margin/2,0]) rotate([0,90,0]) cylinder(h=hallx+margin, d=tubedia);
translate([(hallx+margin)/2,0,hallz+margin]) rotate([-90,0,0]) cylinder(d=wiredia,h=hally+margin*2+tubedia/2);
translate([ziph,0,0]) cube([zipw,ziph,hallz+margin]);
translate([hallx+margin-ziph-zipw,0,0]) cube([zipw,ziph,hallz+margin]);
translate([ziph,tubedia+margin-ziph,0]) cube([zipw,ziph,hallz+margin]);
translate([hallx+margin-ziph-zipw,tubedia+margin-ziph,0]) cube([zipw,ziph,hallz+margin]);


translate([margin+ziph/2,tubedia/2+margin/2-zipw/2,0]) rotate([0,0,90]) cube([zipw,ziph,hallz+margin]);
translate([hallx+margin-zipw-ziph/2,tubedia/2+margin/2-zipw/2,0]) rotate([0,0,90]) cube([zipw,ziph,hallz+margin]);

translate([(hallx+margin)/2-((hallx+margin-zipw-ziph/2)-(margin+ziph/2))/2,tubedia/2+margin/2-zipw/2,tubedia/2-ziph]) cube([(hallx+margin-zipw-ziph/2)-(margin+ziph/2), zipw,ziph*2]);
}
