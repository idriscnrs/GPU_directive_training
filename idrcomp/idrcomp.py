import sys
import os
import json
import matplotlib.pyplot as plt
import re
try:
    import paramiko
except ImportError:
    sys.stderr.write("Paramiko not found. You won't be able to use remote connection")
    pass
from matplotlib import rcParams
from PIL import Image
from subprocess import run, CalledProcessError
from tempfile import NamedTemporaryFile as tmpf
from IPython.core import magic_arguments
from IPython.core.magic import cell_magic, line_magic, Magics, magics_class
import numpy as np
from subprocess import Popen, PIPE

def convert_jpg_to_raw(image_path, output_path):
    # Open image
    image = Image.open(image_path)

    # Convert image to RGB (if it's not already)
    image = image.convert('RGB')

    # Convert image to raw data
    raw_data = np.array(image)

    # Flatten raw data
    raw_data = raw_data.flatten()

    # Save raw data to binary file
    raw_data.tofile(output_path)

def convert_raw_to_jpg(input_path, output_path, height, width):
    # Load raw data from file
    raw_data = np.fromfile(input_path, dtype=np.uint8)

    # Reshape raw data to image size
    image_data = raw_data.reshape((height, width, 3))

    # Convert raw data back to image
    image = Image.fromarray(image_data, 'RGB')

    # Save image
    image.save(output_path)
    
def load_ipython_extension(ipython):
    ipython.register_magics(IdrisMagics)
    
def module_lmod(command, *arguments):
    """ Copied from Lmod repo """
    mod_command = os.environ["LMOD_CMD"]
    proc = Popen([mod_command, 'python', command] + list(arguments), stdout=PIPE, stderr=PIPE, encoding="utf8")
    stdout, stderr = proc.communicate()
    err_out=sys.stderr
    if (os.environ.get('LMOD_REDIRECT','yes') != 'no'):
        err_out=sys.stdout
    exec(stdout)

def convert_pic(filepath, row, col, mode):
    """ Convert raw picture to image"""
    with open(filepath, 'rb') as pic:
        img = Image.frombytes(mode, (col, row), pic.read(), 'raw')
    return img


def show_gray(filepath, row, col):
    """ Display the gray picture in the notebook"""
    img = convert_pic(filepath, row, col, "L")
    display(img)


def show_rgb(filepath, row, col):
    """ Display the RGB: picture in the notebook"""
    img = convert_pic(filepath, row, col, "RGB")
    display(img)


def compare_rgb1(filepath1, filepath2, row, col):
    """ Compare 2 pictures in RGB format """
    img1 = convert_pic(filepath1, row, col, "RGB")
    img2 = convert_pic(filepath2, row, col, "RGB")
    rcParams['figure.figsize'] = 20, 20
    fig, ax = plt.subplots(1, 2)
    fig.tight_layout(pad=3.0)
    ax[0].set_axis_off()
    ax[1].set_axis_off()
    ax[0].imshow(img1)
    ax[1].imshow(img2)
from PIL import Image

def compare_rgb(filepath1, filepath2, boundaries, row, col):
    """ Compare 2 pictures in RGB format """
    img1 = convert_pic(filepath1, row, col, "RGB")
    img2 = convert_pic(filepath2, row, col, "RGB")
    
    top = boundaries[0] * row
    bottom = boundaries[1] * row
    left = boundaries[2] * col
    right = boundaries[3] * col

    img1 = img1.crop((left, top, right, bottom))
    img2 = img2.crop((left, top, right, bottom))

    img1 = img1.resize((col, row))
    img2 = img2.resize((col, row))

    rcParams['figure.figsize'] = 20, 20
    fig, ax = plt.subplots(1, 2)
    fig.tight_layout(pad=3.0)
    ax[0].set_axis_off()
    ax[1].set_axis_off()
    ax[0].imshow(img1)
    ax[1].imshow(img2)

def print_out_err(proc):
    """ Print output and error of a process"""
    stdout = proc.stdout
    stderr = proc.stderr
    if stderr:
        sys.stderr.write(stderr)
        sys.stderr.flush()
    if stdout:
        sys.stdout.write(stdout)
        sys.stdout.flush()


def idrsetenv(modules, lmod=False, remote=False):
    """ Create a copy of an environment after loading some modules """
    # Purge env
    if not remote:
        if lmod:
            module_lmod("purge")
            module_lmod("load", *modules)
            sys.stderr.write("module load {:s}\n".format(" ".join(modules)))
            env = os.environ
        else:
            mod_command = "modulecmd"
            command = [mod_command, "bash", "purge"]
            try:
                proc = run(command, capture_output=True, 
                                    shell=False, 
                                    check=False, 
                                    encoding='utf8')
            except Exception as e:
                sys.stderr.write(f"{' '.join(command)}\n")
                print_out_err(proc)
                raise(e)

            # Load the modules
            command = [mod_command, "bash", "load"]
            command.extend(modules)
            sys.stderr.write("module load {:s}\n".format(" ".join(modules)))

            try:
                proc = run(command, capture_output=True, shell=False, check=False, encoding='utf8')
            except Exception as e:
                print_out_err(proc)
                raise(e)
    # Create a copy of the environment
    # It is used because we would need to reload the modules in each cell
            env = os.environ.copy()
            if not remote:
                for line in proc.stdout.split('\n'):
                    try:
                        var, val = line.split(';')[0].split('=')
                        env[var] = val
                    except ValueError as e:
                        pass
    return env


def receive_file(file_, remote_conf, send_cmd="scp", send_opts=["-pr"], verbose=True):
    """ Receive one file from the remote directory """
    command = [send_cmd, *send_opts, "{:s}@{:s}:{:s}/{:s}".format(remote_conf["user"], remote_conf["host"], remote_conf["directory"], file_), "."]
    if verbose:
        sys.stderr.write(" ".join(command))
        sys.stderr.write("\n")
    try:
        proc = run(command, capture_output=True, shell=False, check=False, encoding='utf8')
    except Exception as e:
        print_out_err(proc)
        raise(e)


def send_files(files, remote_conf, send_cmd="scp", send_opts=["-pr"], verbose=True):
    """ Send files to the remote directory """
    command = [send_cmd, *send_opts, *files, "{:s}@{:s}:{:s}".format(remote_conf["user"], remote_conf["host"], remote_conf["directory"])]
    if verbose:
        sys.stderr.write(" ".join(command))
        sys.stderr.write("\n")
    try:
        proc = run(command, capture_output=True, shell=False, check=False, encoding='utf8')
    except Exception as e:
        print_out_err(proc)
        raise(e)


def setup_ssh(config):
    """ Configure ssh connection """
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(config["host"], username=config["user"])
    return ssh


@magics_class
class IdrisMagics(Magics):
    @cell_magic
    @magic_arguments.magic_arguments()
    @magic_arguments.argument('--compiler', '-c', help='Choose compiler',
                              type=str.lower,
                              choices=['nvhpc', 'gcc', 'pgi', 'intel'], default='nvhpc')
    @magic_arguments.argument('--mpi', '-m',
                              help='Use MPI wrapper and set the number of tasks',
                              type=int,
                              default=0)
    @magic_arguments.argument('--gpus', '-g',
                              help='Set the number of GPUs to use',
                              type=int)
    @magic_arguments.argument('--openmp', '-t', help="Activate OpenMP",
                              action="store_true")
    @magic_arguments.argument('--threads', type=int, default=0, help="Number of OpenMP threads")
    @magic_arguments.argument('--language', '-l', help="Define language",
                              type=str.lower,
                              choices=['c', 'c++','fortran', 'cuda'])
    @magic_arguments.argument('--options',
                              help="Add compiler options. Use quotes to enclose the options",
                              type=str,
                              default=None)
    @magic_arguments.argument('--acc', '-a',
                              help='Activate OpenACC support',
                              action='store_true')
    @magic_arguments.argument('--accopts',
                              type=str,
                              default=None,
                              help="Comma separated options to pass to --ta for PGI, and to --foffload for GCC. Use quotes to enclose the options")
    @magic_arguments.argument('--cliopts',
                              type=str,
                              default=None,
                              help="Options to pass to the command line for the execution. Use quotes to enclose the options")
    @magic_arguments.argument('--get', nargs='+', help='Retrieve list of files')
    @magic_arguments.argument('--notime', action='store_true', help='Remove time information')
    @magic_arguments.argument('--profile', action='store_true', help='Use nsys to generate a profile')
    @magic_arguments.argument('--noexec', '-n', action='store_true', help='Do not execute the code. Requires --keep')
    @magic_arguments.argument('--object', action='store_true', help="Only generate object file. Requires --keep")
    @magic_arguments.argument('--keep', '-k',
                              help='Save source after cell execution. Give the name of the file',
                              type=str,
                              default=None)
    def idrrun(self, line='', cell=None):
        config_file = os.environ["IDR_CONFIG_FILE"]
        self.idrconfig = json.load(open(config_file, 'r'))
        # Check if we use remote launch
        try:
            self.remote_conf = self.idrconfig["remote"]
        except KeyError:
            self.remote_conf = {"enable": False}
        self.is_remote = self.remote_conf["enable"]
        if self.is_remote:
            self.ssh = setup_ssh(self.remote_conf)
        # Check if we are using lmod
        try:
            self.lmod = self.idrconfig["modules"] == "lmod"
        except KeyError:
            self.lmod = False
        self.kind = "cell"
        self.args = magic_arguments.parse_argstring(self.idrrun, line)
        self.keep = self.args.keep
        if self.args.noexec and not self.args.keep:
            #TODO Use a correct Error
            raise BaseException("--noexec requires --keep!!")
        self.initialize()
        self.run_compiler(cell)
        if not self.args.noexec:
            self.run_exec()

    @line_magic
    @magic_arguments.magic_arguments()
    @magic_arguments.argument("file", help="Name of the source file")
    @magic_arguments.argument('--compiler', '-c', help='Choose compiler',
                              type=str.lower,
                              choices=['nvhpc', 'gcc', 'pgi', 'intel'], default='nvhpc')
    @magic_arguments.argument('--mpi', '-m',
                              help='Use MPI wrapper and set the number of tasks',
                              type=int,
                              default=0)
    @magic_arguments.argument('--gpus', '-g',
                              help='Set the number of GPUs to use',
                              type=int)
    @magic_arguments.argument('--get', nargs='+', default=None, help='Retrieve list of files')
    @magic_arguments.argument('--openmp', '-t', help="Activate OpenMP",
                              action="store_true")
    @magic_arguments.argument('--threads', type=int, default=0, help="Number of OpenMP threads")
    @magic_arguments.argument('--language', '-l', help="Define language",
                              type=str.lower,
                              choices=['c', 'c++', 'fortran', 'cuda'])
    @magic_arguments.argument('--options',
                              help="Add compiler options. Use quotes to enclose the options",
                              type=str,
                              default=None)
    @magic_arguments.argument('--acc', '-a',
                              help='Activate OpenACC support',
                              action='store_true')
    @magic_arguments.argument('--accopts',
                              type=str,
                              default=None,
                              help="Comma separated options to pass to --ta for PGI, and to --foffload for GCC. Use quotes to enclose the options")
    @magic_arguments.argument('--cliopts',
                              type=str,
                              default=None,
                              help="Options to pass to the command line for the execution. Use quotes to enclose the options")
    @magic_arguments.argument('--notime', action='store_true', help='Remove time information')
    @magic_arguments.argument('--profile', action='store_true', help='Use nsys to generate a profile')
    @magic_arguments.argument('--noexec', '-n', action='store_true', help='Do not execute the code. Requires --keep')
    @magic_arguments.argument('--object', action='store_true', help="Only generate object file. Requires --keep")
    def idrrunfile(self, line='', cell=None):
        config_file = os.environ["IDR_CONFIG_FILE"]
        self.idrconfig = json.load(open(config_file, 'r'))
        # Check if we use remote launch
        try:
            self.remote_conf = self.idrconfig["remote"]
        except KeyError:
            self.remote_conf = {"enable": False}
        self.is_remote = self.remote_conf["enable"]
        if self.is_remote:
            self.ssh = setup_ssh(self.remote_conf)
        # Check if we are using lmod
        try:
            self.lmod = self.idrconfig["modules"] == "lmod"
        except KeyError:
            self.lmod = False
        self.args = magic_arguments.parse_argstring(self.idrrunfile, line)
        self.kind = "line"
        self.keep = True
        self.initialize()
        self.run_compiler(cell)
        self.file = self.args.file
        if not self.args.noexec:
            self.run_exec()

    def initialize(self):
        """ Setup the run """
        self.default_settings = self.idrconfig["default_settings"]
        self.compiler_opts = []
        self.language = self.args.language if self.args.language else self.default_settings["language"]
        self.nthreads = self.args.threads
        # Check if we are using MPI
        if self.args.mpi > 0:
            self.language = "mpi_"+ self.language
        self.family = self.args.compiler
        self.files_to_retrieve = self.args.get
        # Check if gpus are used
        if not self.args.gpus:
            self.ngpus = int(self.default_settings["gpus"])
        else:
            self.ngpus = self.args.gpus
        # Header to include in source file before compiling
        self.source_header = "\n"
        if self.language == 'c':
            if "c_headers" in self.default_settings.keys():
                for header in self.default_settings["c_headers"]:
                    self.source_header += "#include <{:s}>\n".format(header)
        # Define the options
        opts = self.idrconfig[self.args.compiler]
        # profiler options
        try:
            self.profiling_cmd = self.idrconfig["profiling"]["command"].split()
            self.profiling_opts = self.idrconfig["profiling"]["default_options"]
        except KeyError:
            pass

        # Check if we are passing command line arguments
        self.cli = None
        if self.args.cliopts:
            self.cli = self.args.cliopts[1:-1].split()
        self.compiler = opts[self.language]["compiler"]
        self.extension = opts[self.language]["extension"]
        # Check if we need to load modules
        self.env = os.environ.copy()
        try:
            self.modules = opts["modules"]
            self.env = idrsetenv(self.modules, self.lmod, remote=self.is_remote)
        except KeyError:
            pass
        if self.args.options:
            # the options have to be passed with quotes
            # [1:-1] will remove the quotes
            # TODO add check for quotes
            self.compiler_opts.extend(self.args.options[1:-1].split())

        # Check if we are using OpenACC
        if self.args.acc:
            try:
                with open(opts[self.language]["openacc_header"], 'r') as f:
                    self.source_header += f.read()
            except Exception as e:
                # The language doesn't provide header for openacc
                pass
            self.compiler_opts.append(opts["openacc"]["activate"])
            self.compiler_opts.extend(opts["openacc"]["opts"])
            accopts = []
            if self.args.accopts:
                if self.family == "gcc":
                    for opt in self.args.accopts[1:-1].split(','):
                        accopts.append(opts["openacc"]["accopts_switch"] + opt)
                elif self.family == "pgi":
                    accopts.append(opts["openacc"]["accopts_switch"] + self.args.accopts[1:-1])
                elif self.family == "nvhpc":
                    accopts.append(opts["openacc"]["accopts_switch"] + self.args.accopts[1:-1])
            else:
                accopts.extend([opts["openacc"]["accopts_switch"]+opts["openacc"]["accopts_default"]])
            self.compiler_opts.extend(accopts)
        # Check if we are using OpenMP
        if self.args.openmp:
            self.compiler_opts.append(opts["openmp"]["activate"])


    def run_compiler(self, cell):
        """ Run compiler command, print stdout and stderr"""
        command = []

        command.extend([self.compiler])
        command.extend(self.compiler_opts)
        # Check if we want to keep the source file
        if self.kind == "cell":
            keep = self.keep
            if keep:
                source_file = open(keep, 'w+b')
                self.source = keep
            else:
                # If not create a temporary file
                source_file = tmpf('w+b',
                                   suffix=self.extension,
                                   dir=os.getcwd(),
                                   delete=True)
                self.source = str(source_file.name)
            # Write the content of the cell to the source file
            source_file.write(bytes(self.source_header, "utf-8"))
            source_file.write(bytes(cell, "utf-8"))
            source_file.flush()
        elif self.kind == "line":
            self.source = self.args.file

        # Define the name of the executable or object
        self.exec = os.path.join('.', os.path.basename("{:s}.{:s}".format(self.source, "o" if self.args.object else "exe")))

        # Finish building of command to run
        if self.args.object:
            command.append('-c')
        if self.kind == "line":
            command.extend(['-o', os.path.basename(self.exec), self.source])
        else:
            command.extend(['-o', os.path.basename(self.exec), os.path.basename(self.source)])
        sys.stdout.write(" ".join(command)+"\n")
        # Now run the command
        if self.is_remote:
            send_files([self.source], self.remote_conf)
            command = self.remote_command(command)
            _, stdout, stderr = self.ssh.exec_command(command, get_pty=True)
            # wait for completion
            stdout.channel.recv_exit_status()
            for line in stderr:
                sys.stderr.write(line)
            for line in stdout:
                sys.stdout.write(line)
            receive_file(os.path.basename(self.exec), self.remote_conf)
        else:
            try:
                proc = run(command, capture_output=True, shell=False, check=True, env=self.env, encoding='utf8')
                print_out_err(proc)
                if self.kind == "cell":
                    source_file.close()
            except CalledProcessError as e:
                sys.stderr.write(" ".join(e.cmd))
                sys.stderr.write(e.stderr)
                sys.stderr.write(e.stdout)
                return

    def remote_command(self, command):
        if self.lmod:
            command = "cd {:s}; module load {:s}; {:s}".format(self.remote_conf["directory"], " ".join(self.modules), " ".join(command))
        else:
            command = "cd {:s}; eval $(modulecmd bash load {:s}); {:s}".format(self.remote_conf["directory"], " ".join(self.modules), " ".join(command))
        return command

    def run_exec(self):
        """ Run executable"""
        command = []
        if self.nthreads > 0:
            if self.is_remote:
                command = [f"OMP_NUM_THREADS={self.nthreads}"]
            else:
                os.environ["OMP_NUM_THREADS"]=str(self.nthreads)
        # Get launcher information
        if self.idrconfig["launcher"]["command"]:
            command.append(self.idrconfig["launcher"]["command"])
            command.append(self.idrconfig["launcher"]["tasks"]+str(self.args.mpi if self.args.mpi > 0 else 1))
            command.append(self.idrconfig["launcher"]["threads"]+str(self.nthreads if self.nthreads > 0 else 5))
            if self.ngpus > 0:
                command.append(self.idrconfig["launcher"]["gpus"]+str(self.ngpus))
            command.extend(self.idrconfig["launcher"]["options"])
        # Check if timing is enabled
        if not self.args.notime:
            command.extend(['time', '-f', 'Elapsed time: %E'])
        # use profiler
        if self.args.profile:
            proc = run(["which", "nsys"], capture_output=True, shell=False, check=True, env=self.env, encoding='utf8')
            print_out_err(proc)
            command.extend(self.profiling_cmd)
            if self.profiling_opts:
                command.extend(self.profiling_opts)
        # Add the executable to run
        command.append(os.path.join(".", os.path.basename(self.exec)))
        # Add the command line options
        if self.cli:
            command.extend(self.cli)
        # Now run the command
        if os.path.isfile(self.exec):
            # Make sure that multiword part are enclosed between ""
            print_command = " ".join([f'"{x}"' if len(x.split())>1 else f'{x}' for x in command])
            sys.stdout.write(print_command+"\n")
            if self.is_remote:
                command = self.remote_command(command)
                _, stdout, stderr = self.ssh.exec_command(command)
                # wait for completion
                stdout.channel.recv_exit_status()
                for line in stderr:
                    sys.stderr.write(line)
                for line in stdout:
                    sys.stdout.write(line)
                if self.files_to_retrieve:
                    for f in self.files_to_retrieve:
                        receive_file(f, self.remote_conf)
            else:
                try:
                    proc = run(command, capture_output=True, shell=False, check=True, env=self.env, encoding='utf8')
                    print_out_err(proc)
                except CalledProcessError as e:
                    sys.stderr.write(" ".join(e.cmd))
                    sys.stderr.write(e.stderr)
                    sys.stderr.write(e.stdout)
                    return
        else:
            sys.stderr.write(f"No executable was produce. It should be {self.exec}")
        # Delete executable
        if not self.keep:
            if os.path.exists(self.exec):
                os.remove(self.exec)
