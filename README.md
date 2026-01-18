## Running the Project
Install dependencies:

```
uv sync
```

Then run the following commands at the same time in separate terminals:

```
uv run sam run configs/
```

```
streamlit run src/streamlit_dashboard.py
```

```
python src/main.py
```

## While Running the Project
- `Streamlit` should be opened in your browser. If not, follow instructions in the terminal
- The webcam will turn on after giving permission to access camera
- There is a 5 second detection time
- Afterwards, approximately every 10 seconds, the `Streamlit` page will display the latest results
- If you are not focusing and being distracted, something will happen
