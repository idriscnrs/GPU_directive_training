# OpenACC training course

You should have a folder containing:

- examples/: The examples in C and Fortran with a Makefile in case you do not want or cannot use the code cells in jupyter.
- Fortran/: The notebooks in Fortran
- C/: The notebooks in C
- pictures/: The pictures used in the training course
- idrcomp/: The python packages allowing the compilation and submission of the code cells
- utils/: several utilities necessary to run the training course
  - conda_env.txt: The conda file with the requirements
  - config/: configuration files for some machines

  - start_jupyter_acc.sh: A script to start jupyter with the right configuration for the training

## Setup the environment

1. Download anaconda or miniconda
2. Execute the command in a bash shell

```bash
cd GPU_Directives
conda create --name gpu_directives -c conda-forge --file utils/conda_env.txt
conda activate gpu_directives
```

3. Modify utils/setup_acc.sh to have the right configuration file.

```bash
export IDR_CONFIG_FILE=${conf_dir}/jean-zay.json
```

## Change the default language

You have to modify the language option in the default_settings section of the configuration file:

```json
        "default_settings":{
            "language": "fortran",
        },
```

## How to run the training course remotely

1. Your configuration file needs to have a section like this:

```json
        "remote":{
            "enable": true,
            "host": "<the host where to run the exercises>",
            "user": "<your username>",
            "directory": "/the/working/directory"
        },
```

2. Setup an ssh agent:

```bash
eval $(ssh-agent)
ssh-add
```

We use paramiko to create a connection between your local machine and the host.
And if you do not setup the agent the remote feature will not work properly.

## Using Lmod

If you use Lmod for the modules you need to add to the configuration file:

```json
        "modules": "lmod"
```
