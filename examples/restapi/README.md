This directory contains example scripts for starting the restapi and
interacting with it through a client.

# Steps
1. Install atomspace and cogutil.
2. Install `flask` and `flask-restful`. On Ubuntu, this is
   `sudo apt install python3-flask python3-flask-restful python3-flask-cors`

3. Run the following, replace `/path/to/opencog/clone` with your actual git
   clone path.

   ```
   export PYTHONPATH="${PYTHONPATH}:/usr/lib/python3/dist-packages"
   ```

4. Start the restapi. There are two options use either one.
   * run `python start_restapi.py`
   * run `guile -l start-restapi.scm`
5. In a separate terminal run `python exampleclient.py` for interacting with
   the atomspace.
