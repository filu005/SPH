#version 330 core

in vec4 posout;
// in vec3 tex_coord;

out vec4 color;

void main()
{
    // color = vec4(tex_coord, 1.0);
    //dokladnie ten sam efekt mozna uzyskac skalujac wspolrzedne wierzcholkow do [0;1]
    color = vec4(((posout.x / posout.w) + 1.0f ) / 2.0f,
                ((posout.y / posout.w) + 1.0f ) / 2.0f,
                ((posout.z / posout.w) + 1.0f ) / 2.0f,
                1.0f );
}