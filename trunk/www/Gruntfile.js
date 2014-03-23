module.exports = function(grunt) {
    'use strict';

    // Project configuration.
    grunt.initConfig({
        dirs: {
            dest: 'dist',
            src: 'source'
        },

        // Copy files that don't need compilation to dist/
        copy: {
            dist: {
                files: [
                    {dest: '<%= dirs.dest %>/', src: ['*', '!*.html'], filter: 'isFile', expand: true, cwd: '<%= dirs.src %>/'},
                    {dest: '<%= dirs.dest %>/', src: '.htaccess', expand: true, cwd: '<%= dirs.src %>/'},
                    {dest: '<%= dirs.dest %>/', src: ['img/**', '!**/_old/**'], expand: true, cwd: '<%= dirs.src %>/'},
                    {dest: '<%= dirs.dest %>/', src: 'js/*.min.js', expand: true, cwd: '<%= dirs.src %>/'}
                ]
            }
        },

        includereplace: {
            dist: {
                options: {
                    globals: {
                        metaDescription: '',
                        metaKeywords: ''
                    }
                },
                files: [
                    {src: '*.html', dest: '<%= dirs.dest %>/', expand: true, cwd: '<%= dirs.src %>/'}
                ]
            }
        },

        concat: {
            css: {
                src: ['<%= dirs.src %>/css/normalize.css',
                      '<%= dirs.src %>/css/jquery.fancybox.css',
                      '<%= dirs.src %>/css/style.css'
                ],
                dest: '<%= dirs.dest %>/css/pack.css'
            },
            js: {
                src: ['<%= dirs.src %>/js/plugins.js',
                      '<%= dirs.src %>/js/jquery.mousewheel.js',
                      '<%= dirs.src %>/js/jquery.fancybox.js'
                ],
                dest: '<%= dirs.dest %>/js/pack.js'
            }
        },

        cssmin: {
            dist: {
                options: {
                    keepSpecialComments: 0,
                    report: 'min',
                    compatibility: 'ie8'
                },
                files: {
                    '<%= concat.css.dest %>': '<%= concat.css.dest %>'
                }
            }
        },

        uglify: {
            options: {
                compress: {
                    warnings: false
                },
                mangle: true,
                preserveComments: false,
                report: 'min'
            },
            dist: {
                files: {
                    '<%= concat.js.dest %>': '<%= concat.js.dest %>'
                }
            }
        },

        htmlmin: {
            dist: {
                options: {
                    removeComments: true,
                    collapseWhitespace: true
                },
                expand: true,
                cwd: '<%= dirs.dest %>',
                dest: '<%= dirs.dest %>',
                src: ['**/*.html']
            }
        },

        connect: {
            server: {
                options: {
                    base: '<%= dirs.dest %>/',
                    port: 8001
                }
            }
        },

        watch: {
            options: {
                livereload: true
            },
            files: ['<%= dirs.src %>/**/*', '.csslintrc', '.jshintrc', 'Gruntfile.js'],
            tasks: 'dev'
        },

        clean: {
            dist: '<%= dirs.dest %>/'
        },

        csslint: {
            options: {
                csslintrc: '.csslintrc'
            },
            src: [
                '<%= dirs.src %>/css/style.css'
            ]
        },

        jshint: {
            options: {
                jshintrc: '.jshintrc'
            },
            grunt: {
                src: ['Gruntfile.js', '<%= dirs.src %>/js/plugins.js']
            }
        },

        validation: {
            options: {
                charset: 'utf-8',
                doctype: 'HTML5',
                failHard: true,
                reset: true
            },
            files: {
                src: '<%= dirs.dest %>/**/*.html'
            }
        }

    });

    // Load any grunt plugins found in package.json.
    require('load-grunt-tasks')(grunt, {scope: 'devDependencies'});
    require('time-grunt')(grunt);

    grunt.registerTask('dev', [
        'clean',
        'copy',
        'includereplace',
        'concat'
    ]);

    grunt.registerTask('build', [
        'dev',
        'htmlmin',
        'cssmin',
        'uglify'
    ]);

    grunt.registerTask('test', [
        'build',
        'validation',
        'csslint',
        'jshint'
    ]);

    grunt.registerTask('default', [
        'dev',
        'connect',
        'watch'
    ]);

};