#include "Shader.h"

char shaderContents[2][1024] = {0};
char newShaderContents[2][1024] = {0};

Shader_class::Shader_class(const char* vertexFile, const char* fragmentFile) {
    //stolen from yt video
	char* vertexSource = get_file_contents(vertexFile);
	char* fragmentSource = get_file_contents(fragmentFile);

    setShader(vertexSource, fragmentSource);
    free(vertexSource);
    free(fragmentSource);

}

void Shader_class::setShader(char* vertexSource, char* fragmentSource){
//TODO: make this take in a list of char*s
    int compiled;
    char infoLog[512];
	vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader, 1, &vertexSource, NULL); 
	glCompileShader(vertexShader); 

    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&compiled);

    if(!compiled){
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(1);
    }


	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL); 
	glCompileShader(fragmentShader); 

    glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&compiled);

    if(!compiled){
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return;
        
    }

	shaderID = glCreateProgram(); 
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);

	glLinkProgram(shaderID);
	glDeleteShader(vertexShader); 
	glDeleteShader(fragmentShader);
}

void Shader_class::changeShader(char* vertexSource, char* fragmentSource){
    int compiled;
    char infoLog[512];

	GLuint vertexShader1 = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader1, 1, &vertexSource, NULL); 
	glCompileShader(vertexShader1); 

    glGetShaderiv(vertexShader1,GL_COMPILE_STATUS,&compiled);

    if(!compiled){
    glGetShaderInfoLog(vertexShader1, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return;
        
    }

	GLuint fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader1, 1, &fragmentSource, NULL); 
	glCompileShader(fragmentShader1); 

    glGetShaderiv(fragmentShader1,GL_COMPILE_STATUS,&compiled);

    if(!compiled){
    glGetShaderInfoLog(fragmentShader1, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return;
        
    }
	glDetachShader(shaderID, vertexShader);
	glDetachShader(shaderID, fragmentShader);

    vertexShader = vertexShader1;
    fragmentShader = fragmentShader1;

	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);

	glLinkProgram(shaderID);
	glDeleteShader(vertexShader); 
	glDeleteShader(fragmentShader);
}


char* Shader_class::get_file_contents(const char* filename) {
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

void Shader_class::useShader() {
	glUseProgram(shaderID);
}

void Shader_class::Delete() {
	glDeleteProgram(shaderID);
}
