#!/bin/sh

calibre -query svdb < query_cmd
StarXtract star_cmd
rm -r -f svdb/*
