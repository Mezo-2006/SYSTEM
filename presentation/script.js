(function(){
  const slides = Array.from(document.querySelectorAll('.slide'));
  const prevBtn = document.getElementById('prev');
  const nextBtn = document.getElementById('next');
  const progress = document.getElementById('progress');
  let idx = 0;
  function show(i){
    slides.forEach(s=>s.classList.remove('active'));
    slides[i].classList.add('active');
    progress.textContent = `${i+1} / ${slides.length}`;
  }
  function next(){ idx = Math.min(slides.length-1, idx+1); show(idx); }
  function prev(){ idx = Math.max(0, idx-1); show(idx); }
  prevBtn.addEventListener('click', prev);
  nextBtn.addEventListener('click', next);
  document.addEventListener('keydown', e=>{
    if(e.key === 'ArrowRight' || e.key === 'PageDown') next();
    if(e.key === 'ArrowLeft' || e.key === 'PageUp') prev();
    if(e.key.toLowerCase() === 'h') alert('Navigation: ← → or click buttons. Customize slides in presentation/');
  });
  document.getElementById('deck').addEventListener('click', e=>{ if(e.target.closest('.controls')) return; next(); });
  show(0);
})();
