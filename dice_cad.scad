$fn = 40;

rad = 45;
wall_thickness = 2;
battery_thickness = 5.5;
battery_height = 15;
battery_side = 3;
accel_height = 8.5;
accel_offset = 5;
accel_depth = 15;
accel_width = 20;
accel_side = 2;
accel_pin_height = 7.5;
accel_pin_hole_distance_from_edge = 9.5;
accel_pin_top_diameter = 2;
accel_pin_bottom_diameter = 2;
main_inner_width = 16;
main_outer_width = 25.4;
main_height = 10;
main_rise = 2;
main_thickness = 5;
main_support_distance = 4;
main_support_height = 22;
main_bar_height = 24;
main_bar_width = main_outer_width + 2 * wall_thickness;
main_bar_distance = 0;//1.5;
main_side = 0;
magnet_radius = 5.2;
magnet_height = 3;
magnet_distance = 2.5;
magnet_screw_radius = 1;
magnet_screw_height = 10;
magnet_cylinder_radius = 7.5;

r_outer = rad / sqrt(3);
r_inner = (rad - wall_thickness) / sqrt(3);
angle = 31.717474411;
angle2 = 37.3773681;
angle3 = 116.5650512;
accel_wall_dist = max(0, accel_depth - accel_height * tan( angle3 - 90));

phi = 1 / 2 + sqrt(5 / 4);

wall_p = [[-1,-1,-1] * r_outer,  
[-1,-1,1] * r_outer, 
[-1,1,-1] * r_outer, 
[-1,1,1] * r_outer, 
[1,-1,-1] * r_outer, 
[1,-1,1] * r_outer, 
[1,1,-1] * r_outer, 
[1,1,1] * r_outer,
[0,           1/phi,   phi] * r_outer, 
[0,          -1/phi,   phi] * r_outer,
[0,          1/phi,   -phi] * r_outer, 
[0,          -1/phi, -phi] * r_outer,
[1/phi,   phi,     0] * r_outer,
[-1/phi, phi,    0] * r_outer,
[1/phi,   -phi,  0] * r_outer,
[-1/phi, -phi,  0] * r_outer,
[phi,       0,      1/phi] * r_outer,
[-phi,     0,      1/phi] * r_outer,
[phi,       0,      -1/phi] * r_outer,
[-phi,     0,      -1/phi] * r_outer];

wall_p2 = [[-1,-1,-1] * r_inner,  
[-1,-1,1] * r_inner, 
[-1,1,-1] * r_inner, 
[-1,1,1] * r_inner, 
[1,-1,-1] * r_inner, 
[1,-1,1] * r_inner, 
[1,1,-1] * r_inner, 
[1,1,1] * r_inner,
[0,           1/phi,   phi] * r_inner, 
[0,          -1/phi,   phi] * r_inner,
[0,          1/phi,   -phi] * r_inner, 
[0,          -1/phi, -phi] * r_inner,
[1/phi,   phi,     0] * r_inner,
[-1/phi, phi,    0] * r_inner,
[1/phi,   -phi,  0] * r_inner,
[-1/phi, -phi,  0] * r_inner,
[phi,       0,      1/phi] * r_inner,
[-phi,     0,      1/phi] * r_inner,
[phi,       0,      -1/phi] * r_inner,
[-phi,     0,      -1/phi] * r_inner];

wall_f = [[8,9,1,17,3],[9,5,14,15,1],[9,8,7,16,5],[8,3,13,12,7],[3,17,19,2,13],[17,1,15,0,19]];
wall_f2 = [[3+20,17+20,1+20,9+20,8 + 20],
[1 + 20,15 + 20,14 + 20,5 + 20,9 + 20],
[5 + 20,16 + 20,7 + 20,8 + 20,9 + 20],
[7 + 20,12 + 20,13 + 20,3 + 20,8 + 20],
[13 + 20,2 + 20,19 + 20,17 + 20,3 + 20],
[19 + 20,0 + 20,15 + 20,1 + 20,17 + 20]];

wall_f3 = [[19,0,0+20,19+20],[2,19,19+20,2+20], [13,2,2+20,13+20],[12,13,13+20,12+20],[7,12,12+20,7+20],[16,7,7+20,16+20],[5,16,16+20,5+20],[14,5,5+20,14+20],[15,14,14+20,15+20],[0,15,15+20,0+20]];

battery_p = [
    [-cos(angle) * r_inner + battery_thickness,sin(angle2) * r_inner + battery_thickness * tan(20), 0],
    [-cos(angle) * r_inner + battery_thickness,-sin(angle2) * r_inner - battery_thickness * tan(20), 0],
    [-cos(angle) * r_inner + battery_thickness + wall_thickness,sin(angle2) * r_inner + (battery_thickness + wall_thickness) * tan(20), 0],
    [-cos(angle) * r_inner + battery_thickness + wall_thickness,-sin(angle2) * r_inner - (battery_thickness + wall_thickness) * tan(20), 0],
    [-cos(angle) * r_inner + battery_thickness - battery_height * tan(angle3 - 90),sin(angle2) * r_inner + battery_thickness * tan(18) + battery_height * tan(angle3  - 90 - 1), battery_height],
    [-cos(angle) * r_inner + battery_thickness - battery_height * tan(angle3 - 90),-sin(angle2) * r_inner - battery_thickness * tan(18) - battery_height * tan(angle3  - 90 - 1), battery_height],
    [-cos(angle) * r_inner + battery_thickness + wall_thickness - battery_height * tan(angle3 - 90),sin(angle2) * r_inner + (battery_thickness + wall_thickness) * tan(18) + battery_height * tan(angle3  - 90 - 1), battery_height],
    [-cos(angle) * r_inner + battery_thickness + wall_thickness - battery_height * tan(angle3 - 90),-sin(angle2) * r_inner - (battery_thickness + wall_thickness) * tan(18) - battery_height * tan(angle3  - 90 - 1), battery_height]
];
battery_f = [[0,1,3,2],[5,4,6,7],[2,3,7,6],[1,0,4,5],[0,2,6,4],[3,1,5,7]];

accel_p = [
    [-cos(angle) * r_inner,-accel_width / 2 + accel_offset ,0],
    [-cos(angle) * r_inner,accel_width / 2 + accel_offset,0],
    [-cos(angle) * r_inner + accel_wall_dist,-accel_width / 2 + accel_offset ,0],
    [-cos(angle) * r_inner + accel_wall_dist,accel_width / 2 + accel_offset,0],
    [-cos(angle) * r_inner + accel_wall_dist,-accel_width / 2 + accel_offset ,accel_height],
    [-cos(angle) * r_inner + accel_wall_dist,accel_width / 2 + accel_offset,accel_height],
    [-cos(angle) * r_inner - accel_height * tan(angle3 - 90),-accel_width / 2 + accel_offset ,accel_height],
    [-cos(angle) * r_inner - accel_height * tan(angle3 - 90),accel_width / 2 + accel_offset,accel_height]];

accel_f = [[3,1,0,2],[4,5,3,2],[3,5,7,1],[0,1,7,6],[6,4,2,0],[4,6,7,5]];

accel_pin_p = [[-cos(angle) * r_inner + accel_wall_dist - accel_pin_hole_distance_from_edge,-accel_width / 2 + accel_offset + 1.25 + accel_pin_bottom_diameter / 2,accel_height - accel_pin_height],
    [-cos(angle) * r_inner + accel_wall_dist - accel_pin_hole_distance_from_edge, accel_width / 2 + accel_offset - 1.25 - accel_pin_bottom_diameter / 2,accel_height - accel_pin_height]];
    
main_p = [
    [-cos(angle) * r_inner + main_height / cos(angle),main_inner_width / 2,0],
    [-cos(angle) * r_inner + main_height / cos(angle),-main_inner_width / 2,0],
    [-cos(angle) * r_inner + main_height / cos(angle) + main_thickness,main_inner_width / 2,0],
    [-cos(angle) * r_inner + main_height / cos(angle) + main_thickness,-main_inner_width / 2,0],
    [-cos(angle) * r_inner + main_height / cos(angle) - sin(angle) * main_rise,main_inner_width / 2,main_rise],
    [-cos(angle) * r_inner + main_height / cos(angle) - sin(angle) * main_rise,-main_inner_width / 2,main_rise],
    [-cos(angle) * r_inner + main_height / cos(angle) + main_thickness - sin(angle) * main_rise,main_inner_width / 2,main_rise],
    [-cos(angle) * r_inner + main_height / cos(angle) + main_thickness - sin(angle) * main_rise,-main_inner_width / 2,main_rise]];

main_f = [[0,1,3,2], [1,5,7,3], [0,2,6,4], [5,1,0,4], [2,3,7,6], [4,6,7,5]];

main_p2 = [
    [-cos(angle) * r_inner,main_outer_width / 2,0],
    [-cos(angle) * r_inner,main_outer_width / 2 + wall_thickness,0],
    [-cos(angle) * r_inner + main_support_distance,main_outer_width / 2,0],
    [-cos(angle) * r_inner + main_support_distance,main_outer_width / 2 + wall_thickness,0],
    [-cos(angle) * r_inner - sin(angle) * main_support_height,main_outer_width / 2,main_support_height],
    [-cos(angle) * r_inner - sin(angle) * main_support_height,main_outer_width / 2 + wall_thickness,main_support_height],
    [-cos(angle) * r_inner + main_support_distance - sin(angle) * main_support_height,main_outer_width / 2,main_support_height + tan(angle) * main_support_distance],
    [-cos(angle) * r_inner + main_support_distance - sin(angle) * main_support_height,main_outer_width / 2 + wall_thickness,main_support_height + tan(angle) * main_support_distance]];
    
main_f2 = [[1,0,2,3],[0,1,5,4],[5,1,3,7],[2,0,4,6],[6,7,3,2],[4,5,7,6]];

main_p3 = [
    [-cos(angle) * r_inner,-main_outer_width / 2,0],
    [-cos(angle) * r_inner,-main_outer_width / 2 - wall_thickness,0],
    [-cos(angle) * r_inner + main_support_distance,-main_outer_width / 2,0],
    [-cos(angle) * r_inner + main_support_distance,-main_outer_width / 2 - wall_thickness,0],
    [-cos(angle) * r_inner - sin(angle) * main_support_height,-main_outer_width / 2,main_support_height],
    [-cos(angle) * r_inner - sin(angle) * main_support_height,-main_outer_width / 2 - wall_thickness,main_support_height],
    [-cos(angle) * r_inner + main_support_distance - sin(angle) * main_support_height,-main_outer_width / 2,main_support_height + tan(angle) * main_support_distance],
    [-cos(angle) * r_inner + main_support_distance - sin(angle) * main_support_height,-main_outer_width / 2 - wall_thickness,main_support_height + tan(angle) * main_support_distance]];
    
main_f3 = [[1,0,2,3],[0,1,5,4],[5,1,3,7],[2,0,4,6],[6,7,3,2],[4,5,7,6]];

main_p4 = [
    [-cos(angle) * r_inner - sin(angle) * cos(angle) * main_bar_height,main_bar_width / 2,cos(angle) * main_bar_height],
    [-cos(angle) * r_inner - sin(angle) * cos(angle) * main_bar_height,-main_bar_width / 2,cos(angle) * main_bar_height],
    [-cos(angle) * r_inner + main_bar_distance - sin(angle) * cos(angle) * main_bar_height,main_bar_width / 2,cos(angle) * main_bar_height + main_bar_distance * tan(angle)],
    [-cos(angle) * r_inner + main_bar_distance - sin(angle) * cos(angle) * main_bar_height,-main_bar_width / 2,cos(angle) * main_bar_height + main_bar_distance * tan(angle)],
   
    [-cos(angle) * r_inner - sin(angle) * (cos(angle) * main_bar_height + wall_thickness),main_bar_width / 2,cos(angle) * main_bar_height + wall_thickness],
    [-cos(angle) * r_inner - sin(angle) * (cos(angle) * main_bar_height + wall_thickness),-main_bar_width / 2,cos(angle) * main_bar_height + wall_thickness],
    [-cos(angle) * r_inner + main_bar_distance - sin(angle) * (cos(angle) * main_bar_height + main_bar_distance * tan(angle)),main_bar_width / 2,cos(angle) * main_bar_height + wall_thickness + main_bar_distance * tan(angle)],
    [-cos(angle) * r_inner + main_bar_distance - sin(angle) * (cos(angle) * main_bar_height + main_bar_distance * tan(angle)),-main_bar_width / 2,cos(angle) * main_bar_height + wall_thickness + main_bar_distance * tan(angle)]];
 
main_f4 = [[1,0,2,3],[0,1,5,4],[5,1,3,7],[2,0,4,6],[6,7,3,2],[4,5,7,6]];


//translate([-rad - 2,0,0]) {
    union() {
        /*
        translate([0,0,r_outer-r_inner]) {
            rotate([0,0,72*battery_side]) {
                polyhedron(points = battery_p, faces = battery_f, convexity = 10);
            }
        }
        translate([0,0,r_outer-r_inner]) {
            difference() {
                rotate([0,0,72*accel_side]) {
                    polyhedron(points = accel_p, faces = accel_f, convexity = 10);
                }
                union() {
                    rotate([0,0,72*accel_side]) {
                        for (i = accel_pin_p) {
                            translate(i) {
                                cylinder(accel_pin_height + 1,accel_pin_bottom_diameter / 2,accel_pin_top_diameter / 2);
                            }
                        }
                    }
                }
            }
        }
        translate([0,0,(r_outer-r_inner)]) {
            
            rotate([0,0,72 * main_side]) {
                polyhedron(points = main_p, faces = main_f, convexity = 10);
                polyhedron(points = main_p2, faces = main_f2, convexity = 10);
                polyhedron(points = main_p3, faces = main_f3, convexity = 10);
                polyhedron(points = main_p4, faces = main_f4, convexity = 10);

            }
        }*/
        // NOTE (Elias): Here are some hard-coded values
        translate([4.5  ,8,(r_outer - r_inner) * sqrt(3)]) {
            inner_to_inner = 1.3763819 / sqrt(3) * (rad - wall_thickness);
            rotate([0,0,angle2]) {
                difference() {
                    translate([0,0,-1.5]) {
                        cylinder(h = inner_to_inner - 0.5 - magnet_distance / 2, r = 7.5, center = false);
                    }
                    
                    union() {
                        translate([0,0,inner_to_inner - magnet_height - magnet_distance / 2 - 2 - magnet_screw_height]) {
                                cylinder(r = magnet_screw_radius, h = magnet_screw_height + 1, center = false);
                        }
                        translate([0,0,inner_to_inner - magnet_height - magnet_distance / 2 - 2]) {
                                cylinder(r = magnet_radius + 0.25, h = magnet_screw_height + 100, center = false);
                        }
                    }
                
                    /*union() {
                        translate([0,0,inner_to_inner - magnet_height / 2 - magnet_distance / 2 - 2]) {
                            translate([magnet_cylinder_radius,0,0]) {
                                cube([magnet_cylinder_radius * 2,magnet_radius * 2,magnet_height], center = true);
                            }
                            translate([0,0,0]) {
                                cylinder(r = magnet_radius, h = magnet_height, center = true);
                            }
                        }
                    }*/
                }
            }
        }
        translate([0,0,2.7527638409422998/2 * r_outer]) {
            rotate([0,angle + 180,0]) {
                polyhedron(points = concat(wall_p, wall_p2), faces = concat(wall_f, wall_f2, wall_f3), convexity = 10);
            }
        }
    }
//}
/*
translate([rad + 2,0,2.7527638409422998/2 * r_outer]) {
        rotate([0,angle + 180,0]) {
            polyhedron(points = concat(wall_p, wall_p2), faces = concat(wall_f, wall_f2, wall_f3));
        }
    }
*/
