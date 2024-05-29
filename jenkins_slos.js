pipeline {
    agent any
    environment {
            PATH = "/home/good4u/bin/cmake-3.28.3-linux-x86_64/bin:/home/good4u/bin/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin:/home/good4u/bin/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"
    }
    stages {
        stage('Setup') {
            steps {
                // Check Python version
                sh 'python3 --version'

                // Upgrade pip
                sh 'python3 -m pip install --upgrade pip'

                // Install the desired Python module
                sh 'python3 -m pip install kconfiglib'
            }
        }
        stage('Clone slos') {
            steps {
                // Clone the GitHub repository
                git branch: 'master', url: 'https://github.com/chungae9ri/slos.git'
            }
        }
        stage('Clone Submodule') {
            steps {
                // clone littlefs submodule
                sh 'git submodule update --init --recursive'
            }
        }
        
        stage('make Build') {
            steps {
                sh 'whoami'
                //Run genuine make build steps here
                sh 'make clean'
                sh 'make'
                // Run CMake build steps here
                //script {
                //    dir('build') {
                //        sh 'pwd' // This will show 'build' as the current directory
                //        sh 'cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake ..'
                //        sh 'cmake --build .'
                //    }
                //}
            }
        }
        stage('cmake Build for aarch32') {
            steps {
                sh '/bin/bash build-cmake.sh -c'
                sh '/bin/bash build-cmake.sh -a'

            }
        }
        stage('cmake Build for aarch64') {
            steps {
                sh '/bin/bash build-cmake.sh -c'
                sh '/bin/bash build-cmake.sh -l'

            }
        }
        stage('Clone slos-rust') {
            steps {
                // Clone the GitHub repository
                git branch: 'master', url: 'https://github.com/chungae9ri/slos-rust.git'
            }
        }
        stage('make slos-rust Build') {
            steps {
                sh 'whoami'
                //Run genuine make build steps here
                sh 'make clean'
                sh 'make'
            }
        }
    }
    
    // Define a trigger to run the pipeline whenever there is a new commit to the GitHub repository
    triggers {
    //    pollSCM('*/5 * * * *')
    cron('0 14 * * *')
    }
    
    post {
        //always {
            // Pull the latest changes from the GitHub repository
            //git branch: 'master', url: 'https://github.com/chungae9ri/slos.git'
        //}
        success {
            // Run any post-build steps here
            echo "slos CI/CD successfully done"
        }
    }
}

