import streamlit as st
from polynomial_parser.parser import analyze_and_schedule

# הגדרות תצורה כלליות לעמוד
st.set_page_config(page_title="HSS Profiler", layout="wide")

st.title("🛡️ HSS Polynomial Profiler & Scheduler")
st.markdown("הזן פולינום מטרה והגדר את משאבי החומרה כדי לקבל תוכנית ביצוע אופטימלית והערכת ביצועים.")

# אזור הקלטים
with st.container():
    col1, col2 = st.columns([3, 1])
    with col1:
        poly_input = st.text_input("הכנס פולינום:", value="x1*x2 + x3*x4 + x1*x2*x5", help="ניתן להשתמש במספר משתנים, חיבור וכפל.")
    with col2:
        num_cores = st.number_input("מספר ליבות/מעבדים:", min_value=1, max_value=128, value=2)

# כפתור הפעלה ועיבוד הפלט
if st.button("🚀 נתח ותזמן פולינום", use_container_width=True):
    with st.spinner("מנתח מבנה אלגברי ובונה גרף תלויות..."):
        # קריאה לפונקציה מהקובץ החיצוני
        results = analyze_and_schedule(poly_input, num_cores)
        
        if not results['success']:
            st.error(f"שגיאה בפענוח הפולינום: {results['error']}")
        else:
            st.success(f"הפולינום צומצם בהצלחה ל: {results['optimized']}")
            
            st.markdown("---")
            st.subheader("📊 מדדי ביצועים תיאורטיים")
            
            # תצוגת המטריקות
            metric_cols = st.columns(3)
            metric_cols[0].metric(label="Cryptographic Multiplicative Depth", value=results['depth'], help="משפיע על זמן הריצה הכולל של הפולינום")
            metric_cols[1].metric(label="HSS Heavy Cycles", value=results['hss_muls'], help="סייקלים המכילים לפחות פעולת כפל אחת בין שני סודות")
            metric_cols[2].metric(label="Total Hardware Cycles", value=results['total_cycles'], help="סך מחזורי השעון כולל פעולות חינמיות (חיבור/כפל בסקלר)")
            
            st.markdown("---")
            st.subheader("⏱️ תוכנית ביצוע מקבילית (Execution Plan)")
            
            # הצגת שלבי הביצוע בצורה ויזואלית נוחה
            for cycle, step in results['plan']:
                with st.expander(f"Clock Cycle {cycle}", expanded=True):
                    for action in step:
                        st.code(action, language="python")