scale([1.500000, 1.500000, 1.500000]) 
difference() {
    union() {
    translate([0.000000, 0.000000, 5.000000]) 
        color([0.000000, 0.000000, 1.000000]) 
        cube([6.510000, 15.010000, 25.010000], true);
        
    translate([20.000000, 0.000000, 5.000000]) 
        color([1.000000, 0.000000, 0.000000]) 
        cube([6.510000, 15.010000, 25.010000], true);
        
        color([1.000000, 1.000000, 1.000000]) 
        difference() {
            translate([10.000000, 0.000000, 10.000000]) 
            rotate([0.000000, 90.000000, 0.000000]) 
            cube([15.000000, 15.000000, 26.000000], true);
            translate([10.000000, 0.000000, 5.000000]) 
            rotate([90.000000, 0.000000, 0.000000]) 
            cylinder(15.010000, 7.000000, 7.000000, true);
            translate([10.000000, 0.000000, -1.000000]) 
            cube([13.860000, 15.000000, 14.000000], true);
        }
    }
    
    union() {
        translate([10, 0, 4]) rotate([0,90,0])
        cylinder(30, 4, 4, true);
        translate([10, 0, -2])
        cube([30, 7.85, 11.5], true);
    }
}
