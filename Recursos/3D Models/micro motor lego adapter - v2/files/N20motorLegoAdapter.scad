$fn=100; // Model Resolution. Decrease for Speed, Increase for Quality

translate([0,4+9.3,5.6])
rotate([90,0,0])
color([1.0,0.8,0.2])
N20motor();

translate([0,9.3/2,5.6])
rotate([90,0,0])
color([0.3,1,0.5])
N20shaft();

difference(){
    color([1.0,0.3,0.3])
    N20mount();
//    translate([0,0,50/2+5.6])
//    cube([50,50,50],center=true);
}
//translate([0,0,1])
//color([0.5,0.5,1])
//ToggleSwitch();
//ToggleMount();
//ToggleExtension();

module ToggleExtension(){
    difference(){
        union(){
            // Beam Holes
            for(k=[0:1:1]){
                translate([0,0,7.4/2+24+k*8])
                rotate([90,0,0])
                cylinder(d=7.4,h=7.8,center=true);
                if(k>0){
                    translate([0,0,24+k*8])
                    cube([7.4,7.8,8],center=true);
                }
            }
            // Lever Grip
            hull(){
                translate([0,0,7.4/2+24])
                rotate([90,0,0])
                cylinder(d=7.4,h=7.8,center=true);
                translate([0,0,11])
                scale([0.75,1,1])
                cylinder(d=7.8/(sqrt(3)/2),h=0.1,center=true,$fn=6);
            }
        }
        // Pinhole
        for(i=[-1:2:1],k=[-0:1:1]){
            translate([0,0,7.4/2+24+k*8])
            rotate([90,0,0])
            cylinder(d=4.9,h=7.8+0.2,center=true);
            translate([0,i*(8/2-0.8/2),7.4/2+24+k*8])
            rotate([90,0,0])
            cylinder(d=6.25,h=0.8,center=true);
            translate([0,i*(8/2-0.8/2-0.8),7.4/2+24+k*8])
            rotate([i*90,0,0])
            cylinder(d1=6.25,d2=4.9,h=0.8,center=true);
        }
        // Lever Hole
        translate([0,0,15.2])
        cylinder(d2=3.05+0.1,d1=2.7,h=9,center=true);
        translate([0,0,14.5])
        cube([0.2,8,8],center=true);
    }
    
}
module ToggleMount(){
    // Technic Beam
    difference(){
        union(){
            for(i=[-1.5:3:1.5]){
                translate([8*i,0,7.4/2])
                rotate([90,0,0])
                cylinder(d=7.4,h=8+8-0.2,center=true);
            }
            translate([0,0,7.4/2])
            cube([24,8+8-0.2,7.4],center=true);
            for(i=[-1:2:1]){
                
            }
        }
        // Mounting Pin bores
        for(i=[-1:2:1],k=[-1:2:1]){
            translate([i*1.5*8,0,7.4/2])
            rotate([90,0,0])
            union(){
                translate([0,0,0])
                cylinder(d=4.9,h=8+8,center=true);
                translate([0,0,k*8/2+(8-0.8)/2])
                cylinder(d=6.25,h=0.8,center=true);
                translate([0,0,k*8/2-(8-0.8)/2])
                cylinder(d=6.25,h=0.8,center=true);
                translate([0,0,k*8/2+(8-0.8)/2-0.8])
                cylinder(d2=6.25,d1=4.9,h=0.8,center=true);
//                translate([0,0,k*8/2-(8-0.8)/2])
//                #cylinder(d2=6.25,d1=4.9,h=0.8,center=true);
            }
        }
        // Square Relief
        translate([0,0,1/2])
        cube([13.2,13.2,1.1],center=true);
        // Switch Shaft
        translate([0,0,7.4/2])
        cylinder(d=6.2,h=8,center=true);
        
    }
}
module ToggleSwitch(){
    // Box
    translate([0,0,-9.9/2])
    cube([13,13,9.9],center=true);
    // threaded shaft
    translate([0,0,9.1/2])
    cylinder(d=5.8,h=9.1,center=true);
    // toggle
    for(i=[-1:2:1]){
        translate([i*1.6,0,9.1+9.35/2])
        rotate([0,i*10,0])
        cylinder(d1=2.7,d2=3.05,,h=9.35,center=true);
    }
    // Pins
    for(i=[-1:1:1],j=[-1:2:1]){
        translate([i*4.3,j*4.5/2,-9.9-4.3/2])
        cube([0.8,2.1,4.3],center=true);
    }
    
}
module N20mount(){
    clr = 0.15; // clearance
    difference(){
        union(){
            // Technic Beam
            for(i=[-1:2:1]){
                translate([8*i,8/2,5.6])
                rotate([90,0,0])
                cylinder(d=7.4,h=8+8-0.2,center=true);
            }
            translate([0,8/2,5.6])
            cube([16,8+8-0.2,7.4],center=true);
            // Adapter Shaft Support Cylinder
            translate([0,8/2,5.6])
            rotate([90,0,0])
            cylinder(d=10+1.6,h=8+8-0.2,center=true);
            // Gearbox Holder
            hull(){
                translate([0,8/2+8-0.2/2+3/2,5.6])
                cube([12+3.2,3,10+3.2+2*clr],center=true);
                translate([0,0,5.6])
                cube([1,1,1],center=true);
            }
            translate([0,8/2+9.3+0.7+9.0/2,5.6])
            cube([12+3.2,9,10+3.2+2*clr],center=true);
            for(i=[-1:2:1]){
                translate([0,8/2+9.3+0.7+9.0+15/2,5.6])
                hull(){
                    translate([0,15/2+1/2+0.35,i*(10+1.6+2*clr)/2])
                    cube([6,1,1.6],center=true);
                    translate([0,-15/2-1/2,i*(10+1.6+2*clr)/2])
                    cube([12+3.2,1,1.6],center=true);
                }
                hull(){
                    translate([0,8/2+9.3+0.7+9.0+15+0.2,i*(10+0.55)/2+5.6])
                    rotate([-i*65,0,0])
                    
                    cube([6,3.2,0.1],center=true);
                    translate([0,8/2+9.3+0.7+9.0+15+2,i*(10+0.55)/2+5.6])
                    rotate([i*65,0,0])
                    
                    cube([6,3.2,0.1],center=true);
                }
//                translate([0,8/2+9.3+0.7+9.0+15+1.1,i*(10+0.22)/2+5.6])
//                rotate([-i*65,0,0])
//                cube([6,2.5,1.7],center=true);
            }
                
        }
        // N20 shaft adapter bore
        translate([0,(9.3+4)/2,5.6])
        rotate([90,0,0])
        cylinder(d=3+0.15+2*1.6+0.35,h=8+9.3+4,center=true);
        translate([0,-8/2+0.2/2,5.6])
        rotate([90,0,0])
        cylinder(d1=3+0.15+2*1.6+0.35,d2=3+0.15+2*1.6+0.35+0.8,h=0.5,center=true);
        translate([0,9.3+8/2-clr-1/2+4/2,5.6])
        rotate([90,0,0])
        cylinder(d=3+0.15+2*1.6+2+0.35,h=1+2*clr+4,center=true);
        // Mounting Pin bores
        for(i=[-1:2:1],k=[-1:2:1]){
            translate([i*8,0,5.6])
            rotate([90,0,0])
            union(){
                translate([0,0,-8/2])
                cylinder(d=4.9,h=8+8,center=true);
                translate([0,0,k*((8)/2-0.8/2)])
                cylinder(d=6.25,h=0.8,center=true);
                translate([0,0,8/2-0.8/2-0.8])
                cylinder(d2=6.25,d1=4.9,h=0.8,center=true);
                translate([0,0,k*((8)/2-0.8/2)-8])
                cylinder(d=6.25,h=0.8,center=true);
                translate([0,0,8/2-0.8/2-0.8-8])
                cylinder(d2=6.25,d1=4.9,h=0.8,center=true);
            }
        }
        // Gearbox Cutout
        translate([0,8/2+9.3+0.7+9.0/2,5.6])
        rotate([90,0,0])
        linear_extrude(height=9+clr,center=true){
            offset(r=1+clr/2)
            square([12-2+2*clr,10-2+2*clr],center=true);
        }
        // Motor Cutout
//        translate([0,8/2+9.3+0.7+9.0+15/2,5.6])
//        rotate([90,0,0])
//        intersection(){
//            cylinder(d=12+2*clr,h=15+clr,center=true);
//            translate([0,0,0])
//            cube([12+2*clr,10+2*clr,15+clr],center=true);
//        }
    }
}
module N20shaft(){
    clr = 0.15; // clearance
    difference(){
        union(){
            translate([0,0,0])
            cylinder(d=3+clr+2*1.6,h=8+9.3-2*clr,center=true);
            translate([0,0,-(8+9.3-2*clr)/2+1/2])
            cylinder(d=3+clr+2*1.6+2,h=1,center=true);
        }
        // Motor D-Shaft Cutout
        translate([0,0,-8/2])
        difference(){
            cylinder(d=3+clr,h=9.3+clr,center=true);
            translate([0,3/2+3/2-0.5,0])
            cube([3+clr,3+clr,9.3+clr],center=true);
        }
        // Lego Axle "+" Cutout
        translate([0,0,9.3/2])
        intersection(){
            cylinder(d=4.8,h=8+clr,center=true);
            union(){
                cube([4.8+clr,1.8+clr,8+clr],center=true);
                cube([1.8+clr,4.8+clr,8+clr],center=true);
            }
        }
        
    }
}
module N20motor(){
    union(){
        intersection(){
            cylinder(r=1.5,h=9.3);
            translate([0,0.5,9.3/2])
            cube([3,3,9.3],center=true);
        }
        translate([0,0,-0.7])
        cylinder(r=2,h=0.7);
        translate([0,0,-0.7-9])
        linear_extrude(height=9){
            offset(r=1)
            square([12-2,10-2],center=true);
        }
//        translate([0,0,-0.7-9/2])
//        cube([12,10,9],center=true);
        translate([0,0,-0.7-9-15])
        intersection(){
            cylinder(r=6,h=15);
            translate([0,0,15/2])
            cube([12,10,15],center=true);
        }
        translate([0,0,-0.7-9-15-1.2])
        cylinder(r=2.5,h=1.2);
        translate([6.9/2,0,-0.7-9-15-1.8/2])
        cube([0.25,1.5,1.8],center=true);
        translate([-6.9/2,0,-0.7-9-15-1.8/2])
        cube([0.25,1.5,1.8],center=true);
        translate([0,0,-0.7-9-15-1.2-1])
        cylinder(r=0.5,h=1);
//        translate([0,0,-0.7-9-15-1.2-5]) // Long Shaft for encoder
//        cylinder(r=0.5,h=5);
    }
}