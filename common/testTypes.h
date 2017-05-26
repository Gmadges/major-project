#ifndef TESTTYPES_H
#define TESTTYPES_H

enum ReqType { REGISTER_MESH, REQUEST_MESH, INFO_REQUEST, MESH_UPDATE, REQUEST_MESH_UPDATE};

enum EditType { ADD, DEL, EDIT };

// could use mayas own type system for this....its from a earlier time
enum PolyType { CUBE, PIPE, SPHERE, PYRAMID, CYLINDER, TORUS, CONE, PLANE};

#endif