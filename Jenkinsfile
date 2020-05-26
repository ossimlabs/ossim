properties([
    parameters ([
        string(name: 'BUILD_NODE', defaultValue: 'ossim-build', description: 'The build node to run on'),
        string(name: 'MAKE_VERBOSE', defaultValue: 'VERBOSE=true', description: ''),
        booleanParam(name: 'BUILD_WITH_FORTIFY', defaultValue: false, description: 'Check to build and scan source using HP Fortify tool'),
        booleanParam(name: 'SCAN_WITH_SONARQUBE', defaultValue: false, description: 'Check to perform SonarQube analysis'),
        booleanParam(name: 'CLEAN_WORKSPACE', defaultValue: true, description: 'Clean the workspace at the end of the run')
    ]),
    pipelineTriggers([
            [$class: "GitHubPushTrigger"]
    ]),
    [$class: 'GithubProjectProperty', displayName: '', projectUrlStr: 'https://github.com/ossimlabs/ossim'],
    buildDiscarder(logRotator(artifactDaysToKeepStr: '', artifactNumToKeepStr: '3', daysToKeepStr: '', numToKeepStr: '20')),
    disableConcurrentBuilds()
])

node ("${BUILD_NODE}"){
    env.WORKSPACE = pwd()

    try {
        stage( "Download Artifacts" ) {
            env.OPENCV_HOME = "${env.WORKSPACE}/opencv-3.2-minimal-install"
            withCredentials([string(credentialsId: 'o2-artifact-project', variable: 'o2ArtifactProject')]) {
                step ([$class: "CopyArtifact",
                    projectName: o2ArtifactProject,
                    filter: "common-variables.groovy",
                    flatten: true])
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

            load "common-variables.groovy"
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
                    ${ env.WORKSPACE }/ossim/scripts/ossim-build.sh
                    ${ env.WORKSPACE }/ossim/scripts/ossim-install.sh
                """
            }
        }

        stage ( "Publish Nexus" ) {
            withCredentials([[$class: 'UsernamePasswordMultiBinding',
                credentialsId: 'nexusCredentials',
                usernameVariable: 'REPOSITORY_MANAGER_USER',
                passwordVariable: 'REPOSITORY_MANAGER_PASSWORD']]) {
                    dir( "${ env.WORKSPACE }" ) {
                        sh "${ env.WORKSPACE }/ossim/scripts/oms-deploy.sh"
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

    }
    finally {
        stage( "Clean Workspace" ) {
            if ( "${ CLEAN_WORKSPACE }" == "true" )
                step([$class: 'WsCleanup'])
        }
    }

}
