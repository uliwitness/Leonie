#!/bin/bash
cd `dirname "$0"`
headerdoc2html -o docs .
gatherheaderdoc docs