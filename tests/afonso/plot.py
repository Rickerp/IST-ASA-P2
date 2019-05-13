# Instructions:
#
# sudo apt-get install python3-pip
# pip3 install matplotlib
# sudo apt-get install python3-tk
# python3 plot.py --help
#
# Exemplo 1: python3 plot.py ./a.out -m  3 -fl 2 -c 8 -r 200 2000 50 -o=exemplo_1 -p
# Exemplo 2: python3 plot.py ./a.out -m 10 -fl 2 -c 8 -r 200 2000 5 -o=exemplo_2 -p

import argparse
import pickle
import os
import timeit
from math import log10, floor
# Uncomment if GUI not available
# import matplotlib
# matplotlib.use('Agg')
from matplotlib import pyplot
from subprocess import Popen, PIPE, DEVNULL
from multiprocessing import Pool as ThreadPool


def get_input(v, camadas):
    # ./gerador largura regularidade camadas camadaDeCorte fatorDePesos
    # V = largura * camadas
    largura = v // camadas
    regularidade = largura // 2
    camada_de_corte = camadas // 2
    fator_de_pesos = 5

    p = Popen(["./gerador", str(largura), str(regularidade), str(camadas), str(camada_de_corte), str(fator_de_pesos)],
              stdout=PIPE)

    return p.communicate()[0]


class Plot:

    def __init__(self):

        # Style
        self.__line_color = "red"
        self.__dots_color = "blue"
        self.__dot_size = 2
        self.__line_width = 2

        # Config options
        self.__config = {
            "program": None,
            "cores": None,
            "repeat": 1,
            "hide": False,
            "output": True,
            "memory": False,
            "save_test_config": None
        }

        self.__plot_n = 0
        self.__tests = {
            "plots": 1,
            "layers": 10,
            "values": range(0, 1000, 10),
            "inputs": []
        }

    def get_test_desc(self):
        return "Plots: %s; Layers: %s; Values: %s" %\
               (self.__tests["plots"], self.__tests["layers"], self.__tests["values"])

    def set_config(self, obj):
        self.__config = obj

    def set_layers(self, layers):
        self.__tests["layers"] = layers

    def set_values(self, values):
        self.__tests["values"] = values

    def load(self, load_file):
        log("Loading input from %s" % load_file)
        with open(load_file, "rb") as file:
            self.__tests = pickle.load(file)

    def save_test_config(self):
        save_input_file = self.__config["save_test_config"]
        if save_input_file is None:
            return
        log("Saving generated input to %s" % save_input_file)
        with open(save_input_file, "wb") as file:
            pickle.dump(self.__tests, file, protocol=2)

    def draw(self, xs, ys, plot):
        plot.scatter(xs, ys, s=self.__dot_size, edgecolors=self.__dots_color)

    @staticmethod
    def show_number(x, total):
        places = 1 + floor(log10(total))
        return "{:0{p}}".format(x, p=places)

    @staticmethod
    def show_ratio(x, total):
        return "%s/%d" % (Plot.show_number(x, total), total)

    def run_next_test(self):

        values = self.__tests["values"]
        data = self.__tests["inputs"][self.__plot_n - 1]
        plot_memory = self.__config["memory"]

        xs = list(a / 1000 for a in values)
        ys = [[], []]

        size = len(values)
        for i in range(1, size + 1):

            time = space = 0
            for j in range(1, self.__config["repeat"] + 1):

                msg = "Plot: %s; Input: %s; Repeat: %s" % (
                    self.show_ratio(self.__plot_n, self.__tests["plots"]),
                    self.show_ratio(i, size),
                    self.show_ratio(j, self.__config["repeat"])
                )

                log(msg, end='\r')

                start_time = timeit.default_timer()
                program: str = self.__config["program"]

                if not plot_memory:
                    # Run only for time
                    p = Popen(program, stdout=DEVNULL, stdin=PIPE)
                    p.communicate(data[i - 1])
                else:
                    # Get memory and time for linux only
                    p = Popen(["/usr/bin/time", "-f", "%M", program], stdout=DEVNULL, stdin=PIPE, stderr=PIPE)
                    memory = int(p.communicate(data[i - 1])[1])
                    space += memory / 1024

                time += (timeit.default_timer() - start_time) * 1000

            ys[0].append(time / self.__config["repeat"])
            ys[1].append(space / self.__config["repeat"])

        time_plot = pyplot
        fig = pyplot

        if plot_memory:
            fig, (time_plot, space_plot) = pyplot.subplots(nrows=1, ncols=2, figsize=(9, 3))
            fig.tight_layout(pad=2.5)
            self.draw(xs, ys[1], space_plot)

        self.draw(xs, ys[0], time_plot)

        if self.save_plot() is False and not self.__config["hide"]:
            pyplot.show()

        # Clear plot!
        fig.clf()

    def save_plot(self):
        output: str = self.__config["output"]
        # Ignore output
        if output is False:
            return False
        # Ask output
        elif output is True:
            log()
            pyplot.savefig(input("Save as: "))
        # Save to chosen file
        elif type(output) == str:
            ext = ""
            if self.__tests["plots"] > 1:
                ext = "%s" % self.__plot_n
            pyplot.savefig(output + ext)

        return True

    def average(self, res):
        return res[0] / self.__tests["plots"], res[1] / self.__tests["plots"]

    def generate_output(self):

        plots = self.__tests["plots"]
        log("Generating outputs")

        for i in range(1, plots + 1):
            self.__plot_n = i
            self.run_next_test()

        if self.__config["output"] is not True:
            log()

    def generate_input(self):

        values = self.__tests["values"]
        inputs = self.__tests["inputs"] = []
        layers = self.__tests["layers"]
        plots = self.__tests["plots"] = self.__config["plots"]

        thread_pool = ThreadPool(self.__config["cores"])
        for x in range(1, plots + 1):
            log("Generating inputs %s" % self.show_ratio(x, plots), end='\r')
            plot_inputs = thread_pool.starmap(get_input, ((v, layers) for v in values))
            inputs.append(plot_inputs)

        thread_pool.close()
        thread_pool.join()

        log()
        self.save_test_config()


def log(*args, **kwargs):
    print(*args, **kwargs, flush=True)


def parse_args():

    parser = argparse.ArgumentParser()
    parser.add_argument("program", help="Executable file")

    parser.add_argument("-p", "--memory", help="Draw second plot for memory usage", action="store_true")

    parser.add_argument("-c", "--cores", help="Processor cores used to generate the inputs", type=int, default=1)
    parser.add_argument("-m", "--multiple", help="Perform multiple tests for each input", type=int, default=1)
    parser.add_argument("-t", "--tests", help="Perform test a certain quantity of times", type=int, default=1)
    parser.add_argument("-hi", "--hide", help="Ignore plot", action="store_true")

    primary = parser.add_mutually_exclusive_group(required=True)
    primary.add_argument("-l", "--load", help="Load saved input")
    primary.add_argument("-r", "--range", nargs=3, metavar=("Start", "End", "Step"),
                         help="Range of values for plotting variable", type=int)

    parser.add_argument("-fl", "--fix-layers", help="Fix layers for the input", type=int, required=True)

    parser.add_argument("-s", "--save", help="Save generated input")
    parser.add_argument("-o", "--output", help="Output each plot to file", nargs="?", default=False)

    return parser.parse_args()


def main():

    begin = timeit.default_timer()
    args = parse_args()
    plot = Plot()

    if args.memory and os.name != "posix":
        log("Memory plots only available on POSIX systems")
        args.memory = False

    # Flag passed without file name
    if args.output is None:
        args.output = True

    if args.load is None:
        start, end, step = tuple(args.range)
        values = range(start, end, step)
        plot.set_values(values)
        plot.set_layers(args.fix_layers)
    else:
        plot.load(args.load)

    log(plot.get_test_desc())
    log("Multiple     ", args.multiple)
    log("Output file  ", args.output)
    log("Save file    ", args.save)
    log("Cores        ", args.cores)

    plot.set_config({
        "program": args.program,
        "memory": args.memory,
        "hide": args.hide,
        "repeat": args.multiple,
        "cores": args.cores,
        "plots": args.tests,
        "save_test_config": args.save,
        "output": args.output
    })

    log("Starting linear fitting")
    if not args.load:
        plot.generate_input()

    plot.generate_output()
    log("Exiting: t=%ss" % (timeit.default_timer() - begin))


if __name__ == "__main__":
    main()
