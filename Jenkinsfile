properties([
    parameters ([
        string(name: 'BUILD_NODE', defaultValue: 'ossim-build', description: 'The build node to run on'),
        string(name: 'MAKE_VERBOSE', defaultValue: 'VERBOSE=true', description: ''),
        booleanParam(name: 'BUILD_WITH_FORTIFY', defaultValue: false, description: 'Check to build and scan source using HP Fortify tool'),
        booleanParam(name: 'SCAN_WITH_SONARQUBE', defaultValue: false, description: 'Check to perform SonarQube analysis'),
        string(name: 'GIT_PUBLIC_SERVER_URL', defaultValue: 'https://github.com/ossimlabs', description: ''),
        string(name: 'GIT_PRIVATE_SERVER_URL', defaultValue: 'https://github.com/Maxar-Corp', description: ''),
        string(name: 'CREDENTIALS_ID', defaultValue: 'cicdGithub', description: ''),


        booleanParam(name: 'CLEAN_WORKSPACE', defaultValue: true, description: 'Clean the workspace at the end of the run')
    ]),
    pipelineTriggers([
            [$class: "GitHubPushTrigger"]
    ]),
    [$class: 'GithubProjectProperty', displayName: '', projectUrlStr: 'https://github.com/ossimlabs/tlv'],
    disableConcurrentBuilds()
])

node ("${BUILD_NODE}"){

    try {
        stage( "Download Artifacts" ) {
            dir( "ossim-ci" ) {
                git branch: "BRANCH_NAME",
                url: "${ GIT_PRIVATE_SERVER_URL }/ossim-ci.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }

            dir( "${ env.WORKSPACE }" ) {
                step ([$class: "CopyArtifact",
                    projectName: "kakadu-build",
                    filter: "kakadu.tgz",
                    flatten: true])
                    sh "tar xvfz kakadu.tgz"

                step ([$class: "CopyArtifact",
                    projectName: "opencv-build",
                    filter: "opencv-3.2-minimal-install.tgz",
                    flatten: true,
                    target: "${ env.WORKSPACE }/opencv-3.2-minimal-install"])
                    sh """
                        pushd opencv-3.2-minimal-install
                        tar xvfz opencv-3.2-minimal-install.tgz
                        popd
                    """
            }

            load "ossim-ci/jenkins/variables/common-variables.groovy"
            load "ossim-ci/jenkins/variables/ossim-variables.groovy"
        }

        stage( "Checkout" ) {
            dir( "ossim" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-video" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim-video.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-planet" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim-planet.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-gui" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim-gui.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-oms" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim-oms.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-plugins" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim-plugins.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-wms" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PUBLIC_SERVER_URL }/ossim-wms.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
            dir( "ossim-private" ) {
                git branch: "$BRANCH_NAME",
                url: "${ GIT_PRIVATE_SERVER_URL }/ossim-private.git",
                credentialsId: "${ CREDENTIALS_ID }"
            }
        }

        stage( "Build" ) {
            dir( "${ env.WORKSPACE }" ) {
                sh """
                    rm -rf ${ env.WORKSPACE }/build/CMakeCache.txt
                    export PATH=${ PATH }:/opt/HPE_Security/Fortify_SCA_and_Apps_17.20/bin
                    ${ env.WORKSPACE }/ossim-ci/scripts/ossim-build.sh
                    ${ env.WORKSPACE }/ossim-ci/scripts/ossim-install.sh
                """
            }
        }

        stage ( "Publish Nexus" ) {
            withCredentials([[$class: 'UsernamePasswordMultiBinding',
                credentialsId: 'nexusCredentials',
                usernameVariable: 'REPOSITORY_MANAGER_USER',
                passwordVariable: 'REPOSITORY_MANAGER_PASSWORD']]) {
                    dir( "${ env.WORKSPACE }" ) {
                        sh "${ env.WORKSPACE }/ossim-ci/scripts/oms-deploy.sh"
                    }
            }
        }

        stage( "Archive" ) {
            dir( "${ env.WORKSPACE }" ) {
                sh "tar cvfz ossim-install.tgz install"
            }
            dir( "${ env.WORKSPACE }/artifacts" ) {
                sh "mv ${ env.WORKSPACE }/ossim-install.tgz ."
                sh "cp ${ env.WORKSPACE }/ossim-oms/lib/joms-*.jar ."
            }
            archiveArtifacts "artifacts/*"
        }

        if (SCAN_WITH_SONARQUBE == "true" ) {
            stage( 'SonarQube analysis' ) {
                withSonarQubeEnv( "${ SONARQUBE_NAME }" ) {
                    // requires SonarQube Scanner for Gradle 2.1+
                    // It's important to add --info because of SONARJNKNS-281
                    sh "sonar-scanner"
                }
            }
        }

        if ( BUILD_WITH_FORTIFY == "true" ) {
            stage( 'Fortify SCA' ) {
                dir( "${ env.WORKSPACE }/build" ) {
                    sh """
                        export PATH=${ PATH }:/opt/HPE_Security/Fortify_SCA_and_Apps_17.20/bin
                        sourceanalyzer -64 -b ossimlabs -scan -f fortifyResults-ossim.fpr
                        fortifyclient -url ${ HP_FORTIFY_URL } -authtoken ${ HP_FORTIFY_TOKEN } uploadFPR -file fortifyResults-ossim.fpr -project ossim -version 1.0
                    """
                }
            }
        }
    }
    finally {
        stage( "Clean Workspace" ) {
            if ( "${ CLEAN_WORKSPACE }" == "true" )
                step([$class: 'WsCleanup'])
        }
    }

}
