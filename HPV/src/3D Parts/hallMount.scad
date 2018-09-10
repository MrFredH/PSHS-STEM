// Setup detail level on curved surfaces
$fn=40;
// Define the dimensions of the intended sensor.
hallx=15.6;
hally=23.3;
hallz=8;
// Define width and height of holes for zip ties.
zipw=5;
ziph=2;
// The diameter of the sensor wire
wiredia=5;
// The safety margin on the various surfaces.
margin=zipw+2;
// Diameter of the tube that the sensor attaches to.
tubedia=13.3;

difference()
 {
    // The basic outline of the part
    union()
    {
        cube([hallx+margin,hally+margin*2.5+tubedia/2-(hallz+margin)/2,hallz+margin]);
        translate([0,hally+margin*2.5+tubedia/2-(hallz+margin)/2,(hallz+margin)/2]) rotate([0,90,0]) cylinder(d=hallz+margin, h=hallx+margin);
    }
    
    // Cut out recess for the hall effect sensor.
    translate([margin/2,margin*2.5+tubedia/2,margin]) cube([hallx,hally,hallz]);

    // Cutout for the hall sensor zipties 
    translate([0,hally+margin*2.5+tubedia/2-(hallz+margin)/2-hally/2+zipw/2,hallz+margin-ziph]) cube([hallx+margin, zipw, ziph]);
    
    // Hall mount side ziptie cutouts
    translate([0,hally+margin*2.5+tubedia/2-(hallz+margin)/2-hally/2+zipw/2,0]) cube([ziph, zipw, hallx+margin]);
    translate([hallx+margin-ziph,hally+margin*2.5+tubedia/2-(hallz+margin)/2-hally/2+zipw/2,0]) cube([ziph, zipw, hallx+margin]);
    
    // Cutout the recess for the mounting tube
    translate([0,tubedia/2+margin/2,0]) rotate([0,90,0]) cylinder(h=hallx+margin, d=tubedia);
    
    // Cutout the recess for the sensor wire.
    translate([(hallx+margin)/2,-1,hallz+margin-wiredia/2]) rotate([-90,0,0])
    {
        translate([-wiredia/2, -wiredia,-wiredia/2*0])
        {
            cube([wiredia, wiredia, hally+margin*2+tubedia/2]);
        }
        cylinder(d=wiredia,h=hally+margin*2+tubedia/2);
    }
    
    // Tube outside mounting ziptie cutouts.
    translate([ziph,0,0]) cube([zipw,ziph,hallz+margin]);
    translate([hallx+margin-ziph-zipw,0,0]) cube([zipw,ziph,hallz+margin]);
    
    // Tube inside mounting ziptie cutouts.    
    translate([ziph,tubedia+margin-ziph,0]) cube([zipw,ziph,hallz+margin]);
    translate([hallx+margin-ziph-zipw,tubedia+margin-ziph,0]) cube([zipw,ziph,hallz+margin]);

    // Cable ziptie retainer cutouts. 
    translate([margin+ziph/2,tubedia/2+margin/2-zipw/2,0]) rotate([0,0,90]) cube([zipw,ziph,hallz+margin]);
    translate([hallx+margin-zipw-ziph/2,tubedia/2+margin/2-zipw/2,0])rotate([0,0,90]) cube([zipw,ziph,hallz+margin]);
    // Cable ziptie tube clearence channel
    translate([(hallx+margin)/2-((hallx+margin-zipw-ziph/2)-(margin+ziph/2))/2,tubedia/2+margin/2-zipw/2,tubedia/2-ziph]) cube([(hallx+margin-zipw-ziph/2)-(margin+ziph/2), zipw,ziph*2]);

    // Cable ziptie tube clearence channel
    translate([(hallx+margin)/2-((hallx+margin-zipw-ziph/2)-(margin+ziph/2))/2,tubedia/2+margin/2-zipw/2,tubedia-ziph]) cube([(hallx+margin-zipw-ziph/2)-(margin+ziph/2), zipw,ziph*2]);
}