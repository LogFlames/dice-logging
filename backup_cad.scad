rad = 50;
wall_thickness = 2;
r_outer = rad / sqrt(3);
r_inner = (rad - wall_thickness) / sqrt(3);
angle = 31.717474411;

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

battery_p = [[cos(angle), sin(angle)]];



translate([0,0,-2.7527638409422998/2] * r_outer) {
    rotate([0,angle,0]) {
        polyhedron(points = concat(wall_p, wall_p2), faces = concat(wall_f, wall_f2, wall_f3), convexivity = 4);
    }
}

