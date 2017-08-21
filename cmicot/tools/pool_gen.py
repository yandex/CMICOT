#! /usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import random


def ParseArgs():
    parser = argparse.ArgumentParser(description='Post Antirobot release to Nanny')
    parser.add_argument('FeatureCount', type=int)
    parser.add_argument('LineCount', type=int)
    parser.add_argument('--seed', type=int, default=None)

    return parser.parse_args()


def main():
    args = ParseArgs()

    random.seed(args.seed)
    for line in xrange(args.LineCount):
        values = [random.randint(0, 10)] + [random.uniform(0, 5) for _ in xrange(args.FeatureCount)]
        print '\t'.join([str(x) for x in values])

if __name__ == "__main__":
    main()
