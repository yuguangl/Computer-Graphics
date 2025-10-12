#include "Shader.h"

char shaderContents[2][1024] = {0};
char newShaderContents[2][1024] = {0};

Shader::Shader(const char* vertexFile, const char* fragmentFile) {
	char* vertexSource = get_file_contents(vertexFile);
	char* fragmentSource = get_file_contents(fragmentFile);

    setShader(vertexSource, fragmentSource);
    free(vertexSource);
    free(fragmentSource);

}

void Shader::setShader(char* vertexSource, char* fragmentSource){
//TODO: make this take in a list of char*s
	vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader, 1, &vertexSource, NULL); 
	glCompileShader(vertexShader); 

    int compiled;
    char infoLog[512];
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&compiled);

    if(!compiled){
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(1);
    }


	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL); 
	glCompileShader(fragmentShader); 

	shaderID = glCreateProgram(); 
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);

	glLinkProgram(shaderID);
	glDeleteShader(vertexShader); 
	glDeleteShader(fragmentShader);
}

void Shader::changeShader(char* vertexSource, char* fragmentSource){
	glDetachShader(shaderID, vertexShader);
	glDetachShader(shaderID, fragmentShader);

	vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader, 1, &vertexSource, NULL); 
	glCompileShader(vertexShader); 

    int compiled;
    char infoLog[512];
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&compiled);

    if(!compiled){
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(1);
    }


	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL); 
	glCompileShader(fragmentShader); 

	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);

	glLinkProgram(shaderID);
	glDeleteShader(vertexShader); 
	glDeleteShader(fragmentShader);
}


char* Shader::get_file_contents(const char* filename) {
    FILE* fp = fopen(filename,"rb");
    if(fp == NULL){
        perror("failed to open");
    }
    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp,0,SEEK_SET);
    char* contents = (char*)calloc(fsize+1,sizeof(char));
    fread(contents, fsize,sizeof(char), fp);
    fclose(fp);
    return contents;
}

void Shader::useShader() {
	glUseProgram(shaderID);
}

void Shader::Delete() {
	glDeleteProgram(shaderID);
}
