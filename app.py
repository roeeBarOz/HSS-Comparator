import streamlit as st
import pandas as pd
from polynomial_parser.data import Static_Data
from polynomial_parser.parser import analyze_and_schedule
from Measurements.measure_schemes import measure_schemes
import plotly.express as px

st.set_page_config(page_title="HSS Profiler", layout="wide")
st.title("🛡️ HSS Execution Planner & Timer")

# --- Input Phase ---
col1, col2, col3 = st.columns([3, 1, 1])
with col1:
    poly_input = st.text_input("Enter Polynomial:", value="x1*x2 + x3*x4")
with col2:
    num_cores = st.number_input("Number of Cores:", min_value=1, value=2)
with col3:
    global_size = st.number_input("Variable Size (Bits):", min_value=1, value=8)

if st.button("🚀 Analyze Polynomial & Measure Times", use_container_width=True):
    var_sizes = {'default': global_size}
    
    # === Step 1: Get static data from the parser and display it ===
    static_data: Static_Data = analyze_and_schedule(poly_input, num_cores, var_sizes)
    
    if not static_data.get_success():
        st.error(f"Error: {static_data.get_error()}")
    else:
        st.success(f"Polynomial ready for computation: {static_data.get_optimized_poly()}")
        
        st.markdown("### 📊 Static Data (Computation Structure)")
        stat_cols = st.columns(4)
        stat_cols[0].metric("Unique Inputs", static_data.get_num_unique_inputs())
        stat_cols[1].metric("Result Size (Bits)", static_data.get_max_bit_size())
        stat_cols[2].metric("Total Multiplications", static_data.get_op_counts().get_muls())
        stat_cols[3].metric("Required Clock Cycles", static_data.get_plan_len())
        
        with st.expander("View Task Allocation Plan to Cores (DAG)", expanded=False):
            for cycle, step in static_data.get_plan().items():
                st.markdown(f"**Cycle {cycle}:**")
                for action in step:
                    st.code(str(action), language="python")
                    
        st.markdown("---")
        
        # === Step 2: Send data to the measurement function and get times ===
        with st.spinner("Transferring data to C++ for hardware time measurements..."):
            output = measure_schemes(global_size, static_data.get_op_counts().get_muls(),
                                     static_data.get_op_counts().get_add_mems(), static_data.get_num_unique_inputs(), 
                                     static_data.get_plan(), times=4)
            
        # === Step 3: Display the returned times ===
        st.markdown("### ⏱️ Measurement Results (C++)")
        
        comparison_data = []
        if output is None:
            st.error("An error occurred during measurement. Please try again.")
        else:
            for scheme_name, times_dict in output.items():
                comparison_data.append({
                    "Scheme": scheme_name,
                    "Secret Preparation - Offline (ms)": times_dict['offline_ms'],
                    "Computation Time - Online (ms)": times_dict['online_ms'],
                    "Total Execution Time (ms)": times_dict['total_ms']
                })
                        
            df = pd.DataFrame(comparison_data)
             
            # === תצוגת הטבלה נשארת כפי שהייתה (עם השמות המלאים) ===
            st.dataframe(df.style.highlight_min(subset=["Total Execution Time (ms)"], color='lightgreen'), use_container_width=True, hide_index=True)

            st.markdown("<br>", unsafe_allow_html=True)
            st.markdown("### 📊 Visualizing Execution Phases")

            # === פתרון 1: שינוי שמות העמודות לגרפים כדי שלא ייחתכו ===
            df_charts = df.rename(columns={
                "Secret Preparation - Offline (ms)": "Offline Time",
                "Computation Time - Online (ms)": "Online Time"
            })

            # === פתרון 2: הפרדה לשני גרפים זה לצד זה ===
            # אנחנו יוצרים שתי עמודות לגרפים
            chart_col1, chart_col2 = st.columns(2)

            with chart_col1:
                st.markdown("**Offline Phase (Setup & Inputs)**")
                # יצירת גרף לאופליין בלבד
                fig_offline = px.bar(df_charts, x="Scheme", y="Offline Time", 
                                    color="Scheme", text_auto='.2f')
                # הסרת המקרא (Legend) כי הצבע והציר מובנים מאליהם
                fig_offline.update_layout(showlegend=False, xaxis_title=None, yaxis_title="Time (ms)")
                st.plotly_chart(fig_offline, use_container_width=True)

            with chart_col2:
                st.markdown("**Online Phase (Computation)**")
                # יצירת גרף לאונליין בלבד
                fig_online = px.bar(df_charts, x="Scheme", y="Online Time", 
                                    color="Scheme", text_auto='.2f')
                fig_online.update_layout(showlegend=False, xaxis_title=None, yaxis_title="Time (ms)")
                st.plotly_chart(fig_online, use_container_width=True)